/**
 * @file  utils.c
 * @brief Contains utility functions such as single lists of chars and
 *        data type convertion/checking
 *
 * Copyright (c) 2007 Duolabs S.r.l.
 *
 * Changelog:
 * Date    Author      Comments
 * ----------------------------------------------------------------------------
 * 171007  paguilar    Original
 */

/*****************************
 * INCLUDES
 *****************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "duoutils.h"

/*****************************
 * FUNCTION IMPLEMENTATION
 *****************************/

/**
 * @brief Clear a char buffer with '\\0'
 * @param buff The buffer to be cleared
 * @param size The size of the buffer
 */
void reset_buff(char *buff, int size) {
    int i;

    for (i = 0; i < size; i++)
	buff[i] = '\0';
}

/**
 * @brief Convert an integer to a string
 *        The allocated memory should be freed by the caller
 * @param num The number to be converted to string
 * @return The allocated string
 */
char * int2str(int num) {
    int    i,
           cnt = 0,
           minus = 0,
           tmp;
    char   *str;

    if (!num) {
        if ((str = (char *)xmalloc(sizeof(char), 2)) == NULL)
            return NULL;
        *str = 48;
        *(str + 1) = '\0';
        return str;
    }

    if (num < 0) {
        minus = 1;
        num = num * (-1);
    }

    tmp = num;
    while (tmp) {
        tmp = tmp / 10;
        cnt++;
    }

    if (minus)
        cnt++;

    if ((str = (char *)xmalloc(sizeof(char), cnt + 1)) == NULL)
        return NULL;

    reset_buff(str, strlen(str));

    if (minus)
        cnt--;

    str += cnt - 1; 
    for (i = cnt; i > 0; i--) {
         *str = (num % 10) + 48;
         num = num / 10;
         str--;
    }
    if (minus)
        *str = '-';
    else
        str++;

    return str;
}

/**
 * @brief Convert a 'normal' string to a MAC address string
 * @param mac_addr The MAC address string
 * @param str The string to be converted
 * @return 0 if conversion was successful, -1 otherwise
 */
int str2mac(unsigned char *mac_addr, char *str) {
    int  i;
    char high,
         low;
    char *ptr = str;

    memset(mac_addr, 0, 6);

    for (i = 0; i < 6; i++) {
        high = *ptr;
        low = *(ptr + 1);

        if (high >= 48 && high <= 57)
            high -= 48;
        else if (high >= 97 && high <= 102)
            high -= 87;
        else if (high >= 65 && high <= 70)
            high -= 55;
        else
            return -1;

        if (low >= 48 && low <= 57)
            low -= 48;
        else if (low >= 97 && low <= 102)
            low -= 87;
        else if (low >= 65 && low <= 70)
/*        else if (low >= 65 && high <= low) */
            low -= 55;
        else
            return -1;

        mac_addr[i] |= high;
        mac_addr[i] = mac_addr[i] << 4;
        mac_addr[i] |= low;

        ptr = ptr + 3;
    }

    return 0;
}

/**************************************************
 * Simple linked list of chars
 *************************************************/

/**
 * @brief Initialize the single linked list
 * @return A NULL list
 */
List list_new() {
    return (List)NULL;
}

/**
 * @brief Tells if the list is empty
 * @param A single linked list
 * @return 1 if the list is empty, 0 otherwise
 */
int list_empty(List node) {
    return node == NULL ? 1 : 0;
}
    
/**
 * @brief Adds a new element to the end of the single linked list
 * @param node The pointer to the list
 * @param ch The character to be added
 * @return The list with the new element or NULL if it fails
 */
List list_append(List node, char ch) {
    List ptr = NULL,
         aux = NULL; 
    
    if ((ptr = (List)xmalloc(sizeof(struct List_st), 1)) == NULL) {
        return NULL; 
    }   
        
    ptr->ch = ch;
    ptr->next = NULL;
    
    aux = node;
    if (!aux)
        return ptr;

    while (aux->next) {
        aux = aux->next;
    }
    aux->next = ptr;
        
    return node;
}
    
/**
 * @brief Count the number of elements in the list
 * @param node A single linked list
 * @return The number of elements
 */
int list_count(List node) {
    int  counter = 0;
    List ptr = NULL;
    
    ptr = node;
    while (ptr) {
        counter++;
        ptr = ptr->next;
    }

    return counter;
}

/**
 * @brief Convert a list of chars to a string of the size of the length 
 *        of the list plus 1 for the end of string
 * @param node A single linked list
 * @return The string
 */
char * list_2buff(List node) {
    int  i = 0;
    char *buff;
    List list_ptr;

    if ((buff = (char *)xmalloc(sizeof(char), list_count(node) + 1)) == NULL)
        return (char *)NULL;

    reset_buff(buff, list_count(node) + 1);
    list_ptr = node;

    while (list_ptr) {
        buff[i++] = list_ptr->ch;
        list_ptr = list_ptr->next;
    }

    return buff;
}

/**************************************************
 * Data type checking funcs
 *************************************************/

/**
 * @brief Checks if the given string has a bool value
 * @param The string to be checked
 * @return 0 if it's bool, -1 otherwise
 */
short int is_bool(char *str) {
    char *ch;

    ch = str;
    if (strlen(ch) != 1)
        return -1;

    if (*ch != '0' && *ch != '1')
        return -1;

    return 0;
}

/**
 * @brief Checks if the given string is an integer or decimal
 *        Accepts [+-]\\d+\\.{0,1}\\d*
 * @param The string to be checked
 * @return 0 if it's an integer or decimal, -1 otherwise
 */
short int is_number(char *str) {
    char      *ch;
    int       len;
    short int have_dot = 0;

    ch = str;
    len = strlen(str);
    while (len) {
        /* First char can be a minus sign, an addition sign or a digit */
        if (len == strlen(str)) {
            if (*ch != '-' && *ch != '+' && *ch != '.' && !isdigit(*ch))
                return -1;
            if (*ch == '.' && len == 1)
                return -1;
            if (*ch == '.')
                have_dot = 1;
            if (*ch == '-' && len == 1)
                return -1;
        }
        else {
            if (*ch != '.' && !isdigit(*ch))
                return -1;
            if (have_dot && *ch == '.')
                return -1;
            if (!have_dot && *ch == '.')
                have_dot = 1;
        }
        ch++;
        len--;
    }
    return 0;
}


/**
 * @brief Checks if the given string has the form [0-9aA-zZ_-]+
 * @param The string to be checked
 * @return 0 if it has the form, -1 otherwise
 */
short int is_string(char *str) {
    char *ch;
    int           len;

    ch = str;
    len = strlen(str);
    while (len) {
        if (*ch != '-' && *ch != '_' && !isalnum(*ch))
            return -1;
        ch++;
        len--;
    }
    return 0;
}

/**
 * @brief Checks if the given string is an hex number
 *        Accepts 0x\\h+ or 0X\\h+ or \\h+
 * @param str The string to be checked
 * @return 0 if it has the form, -1 otherwise
 */
short int is_hex(char *str) {
    char *ch;
    int  len;

    ch = str;
    len = strlen(str);
    while (len) {
        if (len == (strlen(str) - 1)) {
            if (*ch == 'x' || *ch == 'X') {
                if (*(--ch) != '0')
                    return -1;
                ch++;
            }
            else {
                if (!isxdigit(*ch))
                    return -1;
            }
        }
        else {
            if (!isxdigit(*ch))
                return -1;
        }
        ch++;
        len--;
    }
    return 0;
}

/**
 * @brief Checks if the given string is a valid IP address
 *        Accepts the dotted decimal and hex notations
 * @param str The string to be checked
 * @return 0 if it's a valid IP address, -1 otherwise
 */
short int is_ipaddr(char *str) {
    char *ch,
         class[4];
    int  len,
         have_dot = 3,
         num = 0;

    len = strlen(str);
    if (len < 7 || len > 15)
        return -1;

    // The IP addr is in hex format?
    if (strstr(str, "X") || strstr(str, "x")) {
        if (is_hex(str))
            return 0;
        return -1;
    }

    reset_buff(class, sizeof(class));

    ch = str;
    while (len) {
        // Error if ch is diff than digit and '.'
        if (*ch != '.' && !isdigit(*ch))
            return -1;

        // Error if first ch is '.'
        if (len == strlen(str))
            if (*ch == '.')
                return -1;

        // Error if there are two '.' together
        if (*ch == '.' && *(ch - 1) == '.')
            return -1;

        // We got a digit
        if (isdigit(*ch)) {
            class[num] = *ch;
            num++;
        }
        // We got a '.'
        else {
            if (atoi(class) > 254)
                return -1;
            reset_buff(class, sizeof(class));
            num = 0;
            have_dot--;
        }

        if (num > 3 || have_dot < 0)
            return -1;

        ch++;
        len--;
    }

    if (have_dot)
        return -1;

    // Check host part
    num = atoi(class);
    if (num < 1 || num > 254)
        return -1;

    return 0;
}

/**
 * @brief Checks if the given string is a valid IP netmask
 *        Accepts the dotted decimal and hex notations
 * @param The string to be checked
 * @return 0 if it's a valid IP netmask, -1 otherwise
 * @todo TODO Accepts the /24 format
 */
short int is_netmask(char *str) {
    char *ch,
         class[4];
    int  len,
         have_dot = 3,
         num = 0;

    len = strlen(str);
    if (len < 7 || len > 15)
        return -1;

    /* The netmask is in hex format? */
    if (strstr(str, "X") || strstr(str, "x")) {
        if (is_hex(str))
            return 0;
        return -1;
    }

    reset_buff(class, sizeof(class));

    ch = str;
    while (len) {
        /* Error if ch is diff than digit and '.' */
        if (*ch != '.' && !isdigit(*ch))
            return -1;

        /* Error if first ch is '.' */
        if (len == strlen(str))
            if (*ch == '.')
                return -1;

        /* Error if there are two '.' together */
        if (*ch == '.' && *(ch - 1) == '.')
            return -1;

        /* We got a digit */
        if (isdigit(*ch)) {
            class[num] = *ch;
            num++;
        }
        /* We got a '.' */
        else {
            if (atoi(class) > 255)
                return -1;
            reset_buff(class, sizeof(class));
            num = 0;
            have_dot--;
        }

        if (num > 3 || have_dot < 0)
            return -1;

        ch++;
        len--;
    }

    if (have_dot)
        return -1;

    if ((strlen(class) < 1) || (atoi(class) > 255))
        return -1;

    return 0;
}

/**
 * @brief Checks if the given string is a valid MAC address
 *        Accepts the 00:11:22:33:44:55 and hex notations
 * @param The string to be checked
 * @return 0 if it's a valid IP netmask, -1 otherwise
 * @todo TODO Accepts other format
 */
short int is_macaddr(char *str) {
    char *ch;
    int  len;

    if (is_hex(str))
        return 0;

    len = strlen(str);
    if (len != 17)
        return -1;

    ch = str;
    while (len) {
        if (len % 3) {
            if (!isxdigit(*ch))
                return -1;
        }
        else {
            if (*ch != ':')
                return -1;
        }
        ch++;
        len--;
    }

    return 0;
}

/**
 * @brief Checks if the given string is a range
 *        Accepts [m-n] -> a range from m to n, m < n
 *        [-m-n] -> a range from -m to n
 *        -[m-n] -> a range from -m to -n, -m < -n
 *        where m and n are integer or decimal
 * @param str The string to be checked
 * @return 0 if it's a valid range, -1 otherwise
 */
short int is_range(char *str) {
    char *ch,
         *m,
         *n;
    int  len,
         negative_range = 0,
         negative_value = 0,
         start_p,
         end_p;

    /* Could be just a number */
    if (is_number(str))
        return -1;

    len = strlen(str);
    if (len < 5)
        return -1;

    ch = str;
    /* We must start with '[' or '-' */
    if (*ch != '[' && *ch != '-')
        return -1;

    /* We must finish with ']' */
    if (*(ch + len - 1) != ']')
        return -1;

    /* Do we start with '[\d' or '[\.' */
    if (*ch == '[' && (isdigit(*(ch + 1)) || *(ch + 1) == '.')) {
        ch++;
        len--;
    }
    /* or with '[-' or '-[' */
    else if ((*ch == '[' && *(ch + 1) == '-') || (*ch == '-' && *(ch + 1) == '[')) {
        if (*ch == '-')
            negative_range = 1;
        if (*(ch + 1) == '-')
            negative_value = 1;
        ch = ch + 2;
        len = len - 2;
    }
    else
        return -1;

    /* Get first limit m */
    start_p = len;
    while (*ch != '-') {
        if (!len)
            return -1;
        if (!isdigit(*ch) && *ch != '.')
            return -1;
        ch++;
        len--;
    }
    end_p = len;

    /* Empty m? */
    if (start_p == len)
        return -1;

    if ((m = (char *)xmalloc(sizeof(char), start_p - end_p + 1)) == NULL) {
         fprintf(stderr, "Not enough memory!!!\n");
         return -1;
    }
    reset_buff(m, start_p - end_p + 1);
    strncpy(m, str + strlen(str) - start_p, start_p - end_p);
    if (!is_number(m)) {
        free(m);
        return -1;
    }

    /* Get last limit n */
    ch++;
    len--;
    start_p = len;
    while (*ch != ']') {
        if (!len) {
            free(m);
            return -1;
        }
        if (!isdigit(*ch) && *ch != '.') {
            free(m);
            return -1;
        }
        ch++;
        len--;
    }
    end_p = len;

    /* Empty n? */
    if (start_p == len)
        return -1;

    if ((n = (char *)xmalloc(sizeof(char), start_p - end_p + 1)) == NULL) {
         fprintf(stderr, "Not enough memory!!!\n");
         free(m);
         return -1;
    }
    reset_buff(n, start_p - end_p + 1);
    strncpy(n, str + strlen(str) - start_p, start_p - end_p);
    if (!is_number(n)) {
        free(n); free(m);
        return -1;
    }

    /* Extra invalid chars? */
    if (len > 1) {
        free(m); free(n);
        return -1;
    }

    /* Compare m and n */
    /* fprintf(stderr, "Range: left limit = %lf, right limit = %lf\n", atof(m), atof(n)); */
    if (negative_range) {
        if (atof(m) < atof(n)) {
            free(m); free(n);
            return -1;
        }
    }

    if (!negative_range && !negative_value) {
        if (atof(m) > atof(n)) {
            free(m); free(n);
            return -1;
        }
    }

    free(m); free(n);
    return 0;
}

/**
 * @brief Check if the buffer has a space
 * @param buff The buffer to be checked
 * @return 0 if it has a space, -1 otherwise
 */
short int check_space(char *buff) {
    int i,
        len,
        have_space = 0;

    len = strlen(buff);

    for (i = 0; i < len; i++) {
        if (isspace(buff[i]))
            have_space = 1;
        else if (isalnum(buff[i] && have_space))
            return -1;
    }
    return 0;
}

/**
 * @brief Check if the buffer is a double quoted string
 * @param buff The buffer to be checked
 * @return 0 if it has a space, -1 otherwise
 */
short int have_double_quotes(char *buff) {
    char *start = NULL,
         *end = NULL;

    start = buff;
    end = buff + sizeof(buff);

    if (*start == '"' && *end == '"')
        return 0;

    return -1;
}

/**************************************************
 * Logging funcs
 *************************************************/

/**
 * @brief Prints the log msgs to a log file using a specific format
 *        Eg Tue Nov  9 11:50:13 2004 : func(): line n: Error msg
 * @param func The function name where the error/warn was produced
 * @param line Line number
 * @param type Error level of criticity
 * @param fmt Error variables
 */
void log_error(const char *func, int line, int type, char *fmt, ...) {
    char     	name[NAME_SIZE],
			 	msg_buff[LOG_BUFF + 128],
             	va_buff[LOG_BUFF];
    FILE     	*fd;
    time_t   	timeval;
    va_list  	ap;
	struct stat	statbuf;

    /* Insert time to buffer */
    timeval = time(0);
    sprintf(msg_buff, "%s", ctime(&timeval));
    msg_buff[strlen(msg_buff) - 1] = '\0';
    msg_buff[strlen(msg_buff)] = '\0';

    sprintf(name, "%.200s/%.50s", D_LOG_PATH, D_LOG_FILE);
	
	if (stat(name, &statbuf)) {
		fprintf(stderr, "%s: %s", msg_buff, strerror(errno));
        return;
	}
	if (statbuf.st_size > MAX_LOG_SIZE) {
		fprintf(stderr, "Max logfile reached %d", MAX_LOG_SIZE);
		unlink(name);
	}

    if ((fd = fopen(name, "a")) == NULL) {
        fprintf(stderr, "%s: %s", msg_buff, strerror(errno));
        return;
    }

    /* Insert func and line to buffer */
    switch (type) {
        case L_CRIT:
            sprintf(msg_buff, "%s: CRITICAL: %s():%d: ", msg_buff, func, line);
            break;
        case L_ERR:
            sprintf(msg_buff, "%s: ERROR: %s():%d: ", msg_buff, func, line);
            break;
        case L_WARN:
            sprintf(msg_buff, "%s: WARNING: %s():%d: ", msg_buff, func, line);
            break;
        case L_INFO:
            sprintf(msg_buff, "%s: INFO: %s():%d: ", msg_buff, func, line);
            break;
        default:
            sprintf(msg_buff, "%s: %s():%d: ", msg_buff, func, line);
    }

    /* Insert msg to buffer */
    if (fmt) {
        va_start(ap, fmt);
        vsnprintf(va_buff, sizeof(va_buff), fmt, ap);
        va_end(ap);
        sprintf(msg_buff, "%s%s\n", msg_buff, va_buff);
    }
    else {
        sprintf(msg_buff, "%s: Unspecified error ocurred\n", msg_buff);
    }

    fprintf(fd, "%s", msg_buff);
    fclose(fd);
}

/**************************************************
 * Read/Write from/to a config file
 *************************************************/

/**
 * @brief Read the value of a key inside the given filename
 *        Supports the format: key=value
 *        with any spaces in-between, value can be a double-quoted string
 * @param filename The filename where the key-value pair is saved
 * @param key The key to be read
 * @return The value of the key, NULL if there was an error
 */
char * config_file_param_read(char *filename, char *key) {
    int   ch,
          have_key = 1,
          have_comment = 0,
          ch_cnt = 0,
          line_cnt = 1;
    char  *key_name = NULL,
          *value = NULL;
    FILE  *fd = NULL;
    List  key_list = NULL,
          value_list = NULL;

    if (!filename || !key)
        return NULL;

    if ((fd = fopen(filename, "r")) == NULL) {
        LOG_DUO(L_ERR, "%s", strerror(errno));
        return NULL;
    }
    
    ch = fgetc(fd);

    do {
        /*printf("Read: %c\n", ch); */
        if (ch_cnt == NAME_SIZE) {
            LOG_DUO(L_ERR, "%s:%d: parse error: maximum string size of %d reached\n", 
                filename, line_cnt, NAME_SIZE);
            fclose(fd); return NULL;
        }

        /* Get key */
        if (have_key) {
            if (ch != '=') {
                /* white space chars and empty list */
                if (isspace(ch)) { 
                    if (ch == 10 && !list_empty(key_list)) {
                        LOG_DUO(L_ERR, "%s:%d: parse error: invalid new line\n", filename, line_cnt);
                        fclose(fd); return NULL;
                    }
                    else if (ch == 10)
                        line_cnt++;
                    continue;
                }
                /* [0-9A-Za-z], '-', '_' */
                else if (isalnum(ch) || (ch == '-') || (ch == '_')) {
                    key_list = list_append(key_list, ch);
                    ch_cnt++;
                }
                /* ch is '#', commments */
                else if (ch == '#') {
                    while ((ch = fgetc(fd)) != 10)
                        continue;
                    line_cnt++;
                }

                else {
                    LOG_DUO(L_ERR, "%s:%d: parse error: invalid character\n", filename, line_cnt);
                    fclose(fd); return NULL;
                }
            }
            else {
                /* list not empty */
                if (!list_empty(key_list)) {
                    have_key = 0;
                    ch_cnt = 0;
                }
                else {
                    LOG_DUO(L_ERR, "%s:%d: parse error: key not found\n", filename, line_cnt);
                    fclose(fd); return NULL;
                }
            }
        }
        /* Get value */
        else {
            if (ch != 10) {
                /* sp and tabs */
                if ((ch == 32 || ch == 9))
                    continue;

                else if (ch == '"') {
                    while ((ch = fgetc(fd)) != '"') {
                        /*printf("Read: %c\n", ch); */
                        value_list = list_append(value_list, ch);
                        ch_cnt++;
                    }

                    if (ch == EOF) {
                        LOG_DUO(L_ERR, "%s:%d: parse error: EOF reached\n", 
                            filename, line_cnt);
                        fclose(fd); return NULL;
                    }
                }
                /* [0-9A-Za-z], '_' */
                else if (isalnum(ch) || (ch == '_')) {
                    value_list = list_append(value_list, ch);
                    ch_cnt++;
                }
                /* Comments */
                else if (ch == '#') {
                    have_comment = 1;
                    if (list_empty(value_list)) {
                        LOG_DUO(L_ERR, "%s:%d: parse error: value not found\n", 
                            filename, line_cnt);
                        fclose(fd); return NULL;
                    }
                    while ((ch = fgetc(fd)) != 10)
                        continue;
                    line_cnt++;
                }

                else {
                    LOG_DUO(L_ERR, "%s:%d: parse error: invalid character\n", 
                        filename, line_cnt);
                    fclose(fd); return NULL;
                }
            }

            if ((ch == 10) || (ch != 10 && have_comment)) {
                line_cnt++;

                if (list_empty(key_list) || list_empty(value_list)) {
                    LOG_DUO(L_ERR, "%s:%d: parse error: key/value not found\n", 
                        filename, line_cnt);
                    fclose(fd); return NULL;
                }

                /* Convert the linked lists to strings */
                if ((key_name = list_2buff(key_list)) == NULL) {
                    LOG_DUO(L_ERR, "%s:%d: Could not allocate memory\n", 
                        filename, line_cnt);
                    fclose(fd); return NULL;
                }

                if ((value = list_2buff(value_list)) == NULL) {
                    LOG_DUO(L_ERR, "%s:%d: Could not allocate memory\n", 
                        filename, line_cnt);
                    fclose(fd); xfree(key_name); return NULL;
                }

                /* Check if we have spaces/tabs  */
                if (!check_space(key_name) || !check_space(value)) {
                    if (!have_double_quotes(value)) {
                        LOG_DUO(L_ERR, "%s:%d: Invalid key/value\n", 
                            filename, line_cnt);
                        fclose(fd); xfree(key_name); xfree(value); return NULL;
                    }
                }

                /*printf("key_name: %s, value: %s\n", key_name, value); */
                if (!strcmp(key, key_name)) {
                    xfree(key_name);
                    key_list = list_new();
                    value_list = list_new();
                    fclose(fd);
                    return value;
                }

                ch_cnt = 0;
                have_key = 1;
                have_comment = 0;
                xfree(key_name);
                xfree(value);
                key_list = list_new();
                value_list = list_new();
            }
        }

    } while ((ch = fgetc(fd)) != EOF);

    fclose(fd);
    return NULL;
}

/**
 * @brief Write the value of a key in the given filename
 * 		The key accepts the chars [0-9A-Za-z], '_', '-'
 * 		The value accepts the chars [0-9A-Za-z], '_', '-', '.', '/' 
 * @param filename The filename where the key-value pair will be saved
 * @param key The key
 * @param value The value
 * @return 0 if the key/value were saved, -1 otherwise
 */
int config_file_param_write(char *filename, char *key, char *value) {
    int   ch,
          have_key = 1,
          have_comment = 0,
          ch_cnt = 0,
          line_cnt = 1;
    char  name[NAME_SIZE],
          backup[NAME_SIZE],
          *key_name = NULL,
          *value_name = NULL;
    FILE  *src = NULL,
          *dst = NULL;
    List  key_list = NULL,
          value_list = NULL;

    if (!filename || !key || !value)
        return -1;

    if ((src = fopen(filename, "r")) == NULL) {
        LOG_DUO(L_ERR, "%s", strerror(errno));
        return -1;
    }

    sprintf(backup, "%s.tmp", filename);
    if ((dst = fopen(backup, "w+")) == NULL) {
        LOG_DUO(L_ERR, "fopen(): %s", strerror(errno));
        fclose(src); return -1;
    }
    fprintf(dst, "#\n# Configuration file\n# %s\n#\n\n", filename);

    ch = fgetc(src);

    do {
        /*printf("Read: %c\n", ch); */
        if (ch_cnt == NAME_SIZE + 1) {
            LOG_DUO(L_ERR, "Maximum string size of %d reached while writing file %s:%d\n", 
                filename, line_cnt, NAME_SIZE);
            fclose(src); return -1;
        }

        /* Get key */
        if (have_key) {
            if (ch != '=') {
                /* white space chars and empty list */
                if (isspace(ch)) { 
                    if (ch == 10 && !list_empty(key_list)) {
                        LOG_DUO(L_ERR, "Invalid new line while writing file %s:%d\n", 
                            filename, line_cnt);
                        fclose(src); fclose(dst); return -1;
                    }
                    else if (ch == 10)
                        line_cnt++;
                    continue;
                }
                /* [0-9A-Za-z], '-', '_' */
                else if (isalnum(ch) || (ch == '-') || (ch == '_')) {
                    key_list = list_append(key_list, ch);
                    ch_cnt++;
                }
                /* ch is '#', commments */
                else if (ch == '#') {
                    while ((ch = fgetc(src)) != 10)
                        continue;
                    line_cnt++;
                }

                else {
                    LOG_DUO(L_ERR, "Invalid character while writing file %s:%d\n", 
                        filename, line_cnt);
                    fclose(src); fclose(dst); return -1;
                }
            }
            else {
                /* list not empty */
                if (!list_empty(key_list)) {
                    have_key = 0;
                    ch_cnt = 0;
                }
                else {
                    LOG_DUO(L_ERR, "Key not found while writing file %s:%d\n", 
                        filename, line_cnt);
                    fclose(src); fclose(dst); return -1;
                }
            }
        }
        /* Put value */
        else {
            if (ch != 10) {
                /* sp and tabs */
                if ((ch == 32 || ch == 9))
                    continue;

                else if (ch == '"') {
                    value_list = list_append(value_list, ch);
                    ch_cnt++;

                    while ((ch = fgetc(src)) != '"') {
                        /*printf("Read: %c\n", ch); */
                        value_list = list_append(value_list, ch);
                        ch_cnt++;
                    }

                    value_list = list_append(value_list, ch);
                    ch_cnt++;

                    if (ch == EOF) {
                        LOG_DUO(L_ERR, "EOF reached while writing file %s:%d\n", 
                            filename, line_cnt);
                        fclose(src); fclose(dst); return -1;
                    }
                }
                /* [0-9A-Za-z], '_', '-', '.', '/' */
                else if (isalnum(ch) || (ch == '_') || (ch == '.') || (ch == '-') || (ch == '/')) {
                    value_list = list_append(value_list, ch);
                    ch_cnt++;
                }
                /* Comments */
                else if (ch == '#') {
                    have_comment = 1;
                    if (list_empty(value_list)) {
                        LOG_DUO(L_ERR, "Value not found while writing file %s:%d\n", 
                            filename, line_cnt);
                        fclose(src); fclose(dst); return -1;
                    }
                    while ((ch = fgetc(src)) != 10)
                        continue;
                    line_cnt++;
                }

                else {
                    LOG_DUO(L_ERR, "Invalid character '%d' while writing file %s:%d\n", 
                            ch, filename, line_cnt);
                    fclose(src); fclose(dst); return -1;
                }
            }

            if ((ch == 10) || (ch != 10 && have_comment)) {
                line_cnt++;

                if (list_empty(key_list) || list_empty(value_list)) {
                    LOG_DUO(L_ERR, "Key/value not found while writing file %s:%d\n", 
                        filename, line_cnt);
                    fclose(src); fclose(dst); return -1;
                }

                /* Convert the linked lists to strings */
                if ((key_name = list_2buff(key_list)) == NULL) {
                    LOG_DUO(L_ERR, "Could not allocate memory while writing file %s:%d\n", 
                        filename, line_cnt);
                    fclose(src); fclose(dst); return -1;
                }

                if ((value_name = list_2buff(value_list)) == NULL) {
                    LOG_DUO(L_ERR, "Could not allocate memory while writing file %s:%d\n", 
                        filename, line_cnt);
                    fclose(src); fclose(dst); xfree(key_name); return -1;
                }

                /* Check if we have spaces/tabs  */
                if (!check_space(key_name) || !check_space(value_name)) {
                    if (!have_double_quotes(value_name)) {
                        LOG_DUO(L_ERR, "Invalid key/value while writing file %s:%d\n", 
                            filename, line_cnt);
                        fclose(src); fclose(dst); xfree(key_name); xfree(value_name); return -1;
                    }
                }

                if (strcmp(key, key_name))
                    fprintf(dst, "%s = %s\n", key_name, value_name);
                else
                    fprintf(dst, "%s = %s\n", key_name, value);

                ch_cnt = 0;
                have_key = 1;
                have_comment = 0;
                xfree(key_name);
                xfree(value_name);
                key_list = list_new();
                value_list = list_new();
            }
        }
    } while ((ch = fgetc(src)) != EOF);

    fputs("\n", dst);
    fclose(dst);
    fclose(src);

    sprintf(name, "%s.bak", filename);
    if (rename(filename, name) == -1) {
        LOG_DUO(L_ERR, "%s", strerror(errno));
        return -1;
    }

    if (rename(backup, filename) == -1) {
        LOG_DUO(L_ERR, "%s", strerror(errno));
        return -1;
    }

    return 0;
}

/**************************************************
 * Debug funcs
 *************************************************/

#ifdef D_DEBUG
void dprint(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(FD_ERR, format, args);
    va_end(args);
}
#else
void dprint(const char *format, ...) { }
#endif    /* D_DEBUG */



