/**
 * @file duoutils.h
 * @brief Macros, data types and function declarations for common functionality
 *        such as memory allocation/deallocation, single linked lists,
 *        data type convertion/checking and FIFO buffers
 *
 * Changelog:
 * Date    Author      Comments
 * ------------------------------------------------------------------------
 * 171007  paguilar    Original
 */

#ifndef _DUOUTILS_H
#define _DUOUTILS_H

/*****************************
 * INCLUDES
 *****************************/

#include <stdio.h>

/*****************************
 * MACROS
 *****************************/

#define SOCK  			int				/**< Socket type */
#define MAX_PKT_LEN  	65536  			/**< Max package length */
#define MAX_LOG_SIZE  	4096 * 1000   	/**< Max log file size: 4MB */
#define	LOG_BUFF        1024			/**< Log buffer size */
#define NAME_SIZE       256				/**< String size */
#define D_LOG_PATH		"/var/log"		/**< Path of log file */
#define D_LOG_FILE		"qboxhd.log"	/**< Log file */
#define MAC_STRLEN      18				/**< Size in bytes of a MAC address */

#define LOG_DUO(type, msg...)                           \
    do {                                                \
        log_error(__FUNCTION__, __LINE__, type, msg);   \
    } while (0)

/*****************************
 * DATA TYPES
 *****************************/

enum log_types {
    L_CRIT,      /* critical conditions */
    L_ERR,       /* error conditions    */
    L_WARN,      /* warning conditions  */
    L_INFO       /* informational       */
};

typedef struct List_st *    List;

/**
 * Node of a single linked list of characters
 */
struct List_st {
    char ch;       /**< Node content: a single character */
    List next;     /**< Next node on list*/
};

typedef struct SList_st *       SList;
/**
 * Node of a single linked list
 */
struct SList_st {
    void  *data;   /**< Node content: any type */
    SList next;    /**< Next node on list */
};


/*****************************
 * FUNCTION DECLARATION
 *****************************/

/* Memory allocation/deallocation */
void *    xmalloc(size_t, int);
void *    xrealloc(void *, size_t);
void      xfree(void *);

/* Data type convertion */
void      reset_buff(char *, int);
char *    int2str(int);
int       str2mac(unsigned char *, char *);

/* Simple linked list of chars */
List      list_new();
int       list_empty(List);
List      list_append(List, char);
int       list_count(List);
char *    list_2buff(List);

/* Simple linked list of void* elements */
SList     slist_new();
SList     slist_append(SList, void *);
SList     slist_insert(SList, void *, int);
SList     slist_remove(SList, int);
int       slist_count(SList);
int       slist_get_position(SList, SList);
int       slist_empty(SList);

/* Data type checking funcs */
short int is_bool(char *);
short int is_number(char *);
short int is_string(char *);
short int is_hex(char *);
short int is_ipaddr(char *);
short int is_netmask(char *);
short int is_macaddr(char *);
short int is_range(char *);
short int check_space(char *);
short int have_double_quotes(char *);

/* Logging funcs */
void      log_error(const char *, int, int, char *, ...);

/* Read/Write from/to a config file */
char *    config_file_param_read(char *, char *);
int       config_file_param_write(char *, char *, char *);

/* Debug funcs */
void      dprint(const char *, ...);

#endif	/* _DUOUTILS_H */
