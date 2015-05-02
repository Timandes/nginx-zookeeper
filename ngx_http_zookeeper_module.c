/**
 * Nginx Zookeeper
 *
 * @author Timandes White <timands@gmail.com>
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <zookeeper/zookeeper.h>

static char *ngx_http_zookeeper_path_parser(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_http_zookeeper_host_parser(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_zookeeper_init_module(ngx_cycle_t *cycle);
static void ngx_http_zookeeper_exit_master(ngx_cycle_t *cycle);
static void *ngx_http_zookeeper_create_main_conf(ngx_conf_t *cf);
static char *ngx_http_zookeeper_init_main_conf(ngx_conf_t *cf, void *conf);

// Configurations
typedef struct {
    ngx_str_t host;
    ngx_str_t path;
    char *cHost;
    char *cPath;
    zhandle_t *handle;
} ngx_http_zookeeper_main_conf_t;

// Directives
static ngx_command_t ngx_http_zookeeper_commands[] = {
    {
        ngx_string("zookeeper_path"),
        NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
        ngx_http_zookeeper_path_parser,
        NGX_HTTP_MAIN_CONF_OFFSET,
        offsetof(ngx_http_zookeeper_main_conf_t, path),
        NULL
    },
    {
        ngx_string("zookeeper_host"),
        NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
        ngx_http_zookeeper_host_parser,
        NGX_HTTP_MAIN_CONF_OFFSET,
        offsetof(ngx_http_zookeeper_main_conf_t, host),
        NULL
    },
    ngx_null_command
};

// Context
static ngx_http_module_t ngx_http_zookeeper_module_ctx = {
    NULL,                                  /* preconfiguration */
    NULL,                                  /* postconfiguration */
    ngx_http_zookeeper_create_main_conf,   /* create main configuration */
    ngx_http_zookeeper_init_main_conf,     /* init main configuration */
    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */
    NULL,                                  /* create location configration */
    NULL                                   /* merge location configration */
};

// Module
ngx_module_t ngx_http_zookeeper_module = {
    NGX_MODULE_V1,
    &ngx_http_zookeeper_module_ctx,        /* module context */
    ngx_http_zookeeper_commands,           /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    ngx_http_zookeeper_init_module,        /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    ngx_http_zookeeper_exit_master,        /* exit master */
    NGX_MODULE_V1_PADDING
};

// Parse configuration 
static char *ngx_http_zookeeper_path_parser(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_conf_set_str_slot(cf, cmd, conf);
    return NGX_CONF_OK;
}

// Parse configuration 
static char *ngx_http_zookeeper_host_parser(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_conf_set_str_slot(cf, cmd, conf);
    return NGX_CONF_OK;
}

// Init module
static ngx_int_t ngx_http_zookeeper_init_module(ngx_cycle_t *cycle)
{
    ngx_http_zookeeper_main_conf_t *zmf;
    int status;

    zmf = (ngx_http_zookeeper_main_conf_t *)ngx_get_conf(cycle->conf_ctx, ngx_http_zookeeper_module);
    if (zmf->host.len <= 0) {
        ngx_log_error(NGX_LOG_WARN, cycle->log, 0, "No zookeeper host was given");
        return NGX_OK;
    }
    if (zmf->path.len <= 0) {
        ngx_log_error(NGX_LOG_WARN, cycle->log, 0, "No zookeeper path was given");
        return NGX_OK;
    }
    if (NULL == zmf->cHost) {
        ngx_log_error(NGX_LOG_WARN, cycle->log, 0, "Impossible cHost");
        return NGX_ERROR;
    }
    if (NULL == zmf->cPath) {
        ngx_log_error(NGX_LOG_WARN, cycle->log, 0, "Impossible cPath");
        return NGX_ERROR;
    }

    // init zookeeper
    zmf->handle = zookeeper_init(zmf->cHost, NULL, 10000, 0, NULL, 0);
    if (NULL == zmf->handle) {
        ngx_log_error(NGX_LOG_WARN, cycle->log, 0, "Fail to init zookeeper instance");
        return NGX_OK;
    }

    // create node
    status = zoo_create(zmf->handle, zmf->cPath, NULL, -1, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, NULL, 0);
    if (ZOK != status) {
        ngx_log_error(NGX_LOG_WARN, cycle->log, 0, "Fail to create zookeeper node");
        zookeeper_close(zmf->handle);
        zmf->handle = NULL;
        return NGX_OK;
    }

    return NGX_OK;
}

// Exit master
static void ngx_http_zookeeper_exit_master(ngx_cycle_t *cycle)
{
    ngx_http_zookeeper_main_conf_t *zmf;

    zmf = (ngx_http_zookeeper_main_conf_t *)ngx_get_conf(cycle->conf_ctx, ngx_http_zookeeper_module);
    if (zmf->handle)
        zookeeper_close(zmf->handle);
}

// Create main conf
static void *ngx_http_zookeeper_create_main_conf(ngx_conf_t *cf)
{
    ngx_http_zookeeper_main_conf_t *retval;
    retval = ngx_pcalloc(cf->pool, sizeof(ngx_http_zookeeper_main_conf_t));
    if (NULL == retval)
        return NGX_CONF_ERROR;

    retval->host.len = 0;
    retval->host.data = NULL;
    retval->cHost = NULL;
    retval->path.len = 0;
    retval->path.data = NULL;
    retval->cPath = NULL;
    retval->handle = NULL;
    return retval;
}

// Init main conf
static char *ngx_http_zookeeper_init_main_conf(ngx_conf_t *cf, void *conf)
{
    ngx_http_zookeeper_main_conf_t *mf = conf;

    // host
    if (mf->host.len <= 0)
        ngx_log_stderr(0, "WARNING: No zookeeper host was given");
    else {
        mf->cHost = malloc(mf->host.len + 1);
        if (NULL == mf->cHost) {
            ngx_log_stderr(0, "Fail to malloc for host");
            return NGX_CONF_ERROR;
        }
        memcpy(mf->cHost, mf->host.data, mf->host.len);
        mf->cHost[mf->host.len] = 0;
    }

    // path
    if (mf->path.len <= 0)
        ngx_log_stderr(0, "WARNING: No zookeeper path was given");
    else {
        mf->cPath = malloc(mf->path.len + 1);
        if (NULL == mf->cPath) {
            ngx_log_stderr(0, "Fail to malloc for path");
            return NGX_CONF_ERROR;
        }
        memcpy(mf->cPath, mf->path.data, mf->path.len);
        mf->cPath[mf->path.len] = 0;
    }
    

    return NGX_CONF_OK;
}
