#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "cjson_config.h"

static __u32 calc_crc32(const unsigned char *buffer, __u32 length)
{
	typedef struct tag_CRC32_DATA
	{
	    __u32 CRC;
	    __u32 CRC_32_Tbl[256];
	} CRC32_DATA_t;

	__u32 i, j;
    CRC32_DATA_t crc32;
    __u32 CRC32 = 0xffffffff;
    crc32.CRC = 0;

    for ( i = 0; i < 256; ++i)
    {
        crc32.CRC = i;
        for ( j = 0; j < 8 ; ++j)
        {
            if (crc32.CRC & 1)
                crc32.CRC = (crc32.CRC >> 1) ^ 0xEDB88320;
            else
                crc32.CRC >>= 1;
        }
        crc32.CRC_32_Tbl[i] = crc32.CRC;
    }

    CRC32 = 0xffffffff;
    for ( i = 0; i < length; ++i)
    {
        CRC32 = crc32.CRC_32_Tbl[(CRC32^((unsigned char*)buffer)[i]) & 0xff] ^ (CRC32 >> 8);
    }

    return CRC32^0xffffffff;
}

static int cjson_config_read(cjson_config_t *config, char *config_path) {
	FILE *fp = NULL;
	struct stat buf;

	fp = fopen(config_path, "r");
	if(!fp) {
		com_warn("%s open fail\n", config_path);
		return -1;
	}
	memset(&buf, 0x00, sizeof(buf));
	stat(config_path, &buf);
	if (!buf.st_size){
		com_warn("%s size = 0\n", config->config_path);
		fclose(fp);
		return -1;
	}
	config->config_size = buf.st_size;
	config->config_contents = malloc(config->config_size);
	if(!config->config_contents){
		com_warn("malloc %d fail\n", config->config_size);
		fclose(fp);
		return -1;
	}
	fread(config->config_contents, 1, config->config_size, fp);
	fclose(fp);
	return 0;
}

static int cjson_config_file_write(cjson_config_t *config, char *config_path) {
	FILE *fp = NULL;

	fp = fopen(config_path, "w+");
	if (!fp) {
		perror("fopen fail\n");
		com_warn("%s open fail\n", config_path);
		return 0;
	}
	fwrite(config->config_contents, 1, config->config_size, fp);
	fclose(fp);
	return 0;
}

static int cjson_config_write(cjson_config_t *config) {
	if (config->backup_config_path != NULL) {
		cJSON *root = NULL;
		cJSON *main_item = NULL;
		unsigned int check_sum = 0;
		char *text = NULL;

		if (config->config_contents == NULL) {
			com_err("config->config_path NULL");
			return 0;
		}

		root = cJSON_Parse(config->config_contents);
		if (!root) {
			com_err("cJSON_Parse fail");
			return 0;
		}
		free(config->config_contents);
		config->config_contents = NULL;
		config->config_size = 0;

		main_item = cJSON_GetObjectItem(root, "verify");
		if (!main_item) {
			main_item = cJSON_CreateObject();
			cJSON_AddItemToObject(root, "verify", main_item);
			cJSON_AddNumberToObject(main_item, "check_sum", 0);
		} else {
			cJSON_DeleteItemFromObject(main_item, "check_sum");
			cJSON_AddNumberToObject(main_item, "check_sum", 0);
		}

		text = cJSON_Print(root);
		config->config_contents = text;
		check_sum = calc_crc32((const unsigned char *)config->config_contents, strlen(config->config_contents));
//		com_info("check_sum:0x%x", check_sum);
		cJSON_DeleteItemFromObject(main_item, "check_sum");
		cJSON_AddNumberToObject(main_item, "check_sum", check_sum);
		free(config->config_contents);
		config->config_contents = NULL;
		config->config_size = 0;

		config->config_contents = cJSON_Print(root);
		if(root){
			cJSON_Delete(root);
			root = NULL;
		}
		config->config_size = strlen(config->config_contents);
		cjson_config_file_write(config, config->config_path);
		cjson_config_file_write(config, config->backup_config_path);
	} else {
		cjson_config_file_write(config, config->config_path);
	}
	sync();
	return 0;
}

static int cjson_config_verify(cjson_config_t *config) {
	unsigned int check_sum = 0, actual_check_sum = 0;
	char *text = NULL;
	cJSON *root = NULL;
	cJSON *tmp_item = NULL;
	cJSON *main_item = NULL;

	if (config->config_contents == NULL) {
		com_err("config->config_path NULL");
		return 0;
	}

	root = cJSON_Parse(config->config_contents);
	if (!root) {
		com_err("cJSON_Parse fail");
		return 0;
	}
	free(config->config_contents);
	config->config_contents = NULL;
	config->config_size = 0;

	main_item = cJSON_GetObjectItem(root, "verify");
	if (!main_item) {
		com_err("cJSON_GetObjectItem verify fail");
		goto done;
	} else {
		tmp_item = cJSON_GetObjectItem(main_item, "check_sum");
		if (!tmp_item) {
			com_err("cJSON_GetObjectItem check_sum fail");
			goto done;
		}
		check_sum = tmp_item->valueint;
		com_info("check_sum:0x%x", check_sum);
	}
	cJSON_DeleteItemFromObject(main_item, "check_sum");
	cJSON_AddNumberToObject(main_item, "check_sum", 0);
	text = cJSON_Print(root);
	config->config_contents = text;
	config->config_size = strlen(config->config_contents);

	actual_check_sum = calc_crc32((const unsigned char *)config->config_contents, strlen(config->config_contents));
	com_info("actual_check_sum:0x%x", actual_check_sum);
	if (check_sum != actual_check_sum) {
		com_err("check_sum:0x%x actual_check_sum:0x%x", check_sum, actual_check_sum);
	}
	cJSON_DeleteItemFromObject(main_item, "check_sum");
	cJSON_AddNumberToObject(main_item, "check_sum", check_sum);
	if (config->config_contents) {
		free(config->config_contents);
		config->config_contents = NULL;
		config->config_size = 0;
	}
done:
	text = cJSON_Print(root);
	config->config_contents = text;
	config->config_size = strlen(config->config_contents);
	if (root) {
		cJSON_Delete(root);
		root = NULL;
	}
	return 1;
}
int init_cjson_config(cjson_config_t *config)
{
	int ret = 0;
	config->valid = 0;
	pthread_mutex_init(&config->param_mutex, NULL);
	cjson_config_read(config, config->config_path);
	if (config->backup_config_path != NULL) {
		char cmd_line[256] = {0};
		int cjson_valid = 0, backup_cjson_valid = 0;

		cjson_valid = cjson_config_verify(config);
		if (config->config_contents) {
			free(config->config_contents);
			config->config_contents = NULL;
			config->config_size = 0;
		}

		cjson_config_read(config, config->backup_config_path);
		backup_cjson_valid = cjson_config_verify(config);
		if (config->config_contents) {
			free(config->config_contents);
			config->config_contents = NULL;
			config->config_size = 0;
		}

		if (backup_cjson_valid == 0 && cjson_valid == 1) {
			sprintf(cmd_line, "cp %s %s", config->config_path, config->backup_config_path);
			system(cmd_line);
		} else if (backup_cjson_valid == 1 && cjson_valid == 0) {
			sprintf(cmd_line, "cp %s %s", config->backup_config_path, config->config_path);
			system(cmd_line);
		} else if (backup_cjson_valid == 0 && cjson_valid == 0) {//maybe when init,can happen
			//eed processn
			com_info("backup_cjson_valid == 0 && cjson_valid == 0");
			ret = 1;
			goto done;
		}
		cjson_config_read(config, config->config_path);
	}
	com_info("there");
	config->valid = 1;
done:
	pthread_mutex_unlock(&config->param_mutex);
	return ret;
}
int read_int_type_cjson_config(cjson_config_t *config, char *major, char *minor, int *param)
{
	int ret = 0;
	cJSON *root = NULL;
	cJSON *tmp_item = NULL;
	cJSON *main_item = NULL;

	pthread_mutex_lock(&config->param_mutex);
	if (!config->valid) {
		com_warn("%s not valid\n", config->config_path);
		ret = -1;
		goto Failed;
	}

	if (!config->config_contents) {
		ret = -1;
		goto Failed;
	}
	root = cJSON_Parse(config->config_contents);
	if (!root) {
		ret = -1;
		goto Failed;
	}

	main_item = cJSON_GetObjectItem(root, major);
	if (!main_item) {
		ret = -1;
		goto Failed;
	}
	tmp_item = cJSON_GetObjectItem(main_item, minor);
	if (!tmp_item) {
		ret = -1;
		goto Failed;
	}
	*param = tmp_item->valueint;
Failed:
	if (root) {
		cJSON_Delete(root);
	}
	pthread_mutex_unlock(&config->param_mutex);
	return ret;
}
int write_int_type_cjson_config(cjson_config_t *config, char *major, char *minor, int param)
{
	cJSON *root = NULL;
	cJSON *item = NULL;
	cJSON *new_item = NULL;
	char *text = NULL;

	pthread_mutex_lock(&config->param_mutex);
	if (config->config_contents)
	{
		root = cJSON_Parse(config->config_contents);
		if(!root){
			root = cJSON_CreateObject();
		}
		free(config->config_contents);
		config->config_contents = NULL;
		config->config_size = 0;
	} else {
		root = cJSON_CreateObject();
	}

	item = cJSON_GetObjectItem(root, major);
	if(item)
	{
		cJSON_DeleteItemFromObject(item, minor);
		cJSON_AddNumberToObject(item, minor, param);
	}
	else
	{
		new_item = cJSON_CreateObject();
		cJSON_AddItemToObject(root, major, new_item);
		cJSON_AddNumberToObject(new_item, minor, param);
	}
	//**add check sum
	text = cJSON_Print(root);
	if (root) {
		cJSON_Delete(root);
		root = NULL;
	}
	config->config_contents = text;
	config->config_size = strlen(config->config_contents);
	cjson_config_write(config);
	config->valid = 1;

	pthread_mutex_unlock(&config->param_mutex);
	return 0;
}
int read_string_type_cjson_config(cjson_config_t *config, char *major, char *minor, char *str, int len)
{
	int ret = 0;
	cJSON *root = NULL;
	cJSON *tmp_item = NULL;
	cJSON *main_item = NULL;

	pthread_mutex_lock(&config->param_mutex);

	if (!config->valid) {
		com_warn("%s not valid\n", config->config_path);
		ret = -1;
		goto Failed;
	}

	if (!config->config_contents) {
		ret = -1;
		goto Failed;
	}
	root = cJSON_Parse(config->config_contents);
	if(!root){
		ret = -1;
		goto Failed;
	}
	main_item = cJSON_GetObjectItem(root, major);
	if (!main_item) {
		ret = -1;
		goto Failed;
	}

	tmp_item = cJSON_GetObjectItem(main_item, minor);
	if(!tmp_item) {
		ret = -1;
		goto Failed;
	}
	if (strlen(tmp_item->valuestring) != 0) {
		memcpy(str, tmp_item->valuestring, strlen(tmp_item->valuestring) > len ? len:strlen(tmp_item->valuestring));
	} else {
		ret = -1;
	}
Failed:
	if (root) {
		cJSON_Delete(root);
	}
	pthread_mutex_unlock(&config->param_mutex);
	return ret;
}

int write_string_type_cjson_config(cjson_config_t *config, char *major, char *minor, char *str, int len)
{
	cJSON *root = NULL;
	cJSON *item = NULL;
	cJSON *new_item = NULL;
	char *text = NULL;

	pthread_mutex_lock(&config->param_mutex);
	if (config->config_contents)
	{
		root = cJSON_Parse(config->config_contents);
		if(!root){
			root = cJSON_CreateObject();
		}
		free(config->config_contents);
		config->config_contents = NULL;
		config->config_size = 0;
	} else {
		root = cJSON_CreateObject();
	}

	item = cJSON_GetObjectItem(root, major);
	if(item)
	{
		cJSON_DeleteItemFromObject(item, minor);
		if (str != NULL) {
			cJSON_AddStringToObject(item, minor, str);
		}
	}
	else
	{
		new_item = cJSON_CreateObject();
		cJSON_AddItemToObject(root, major, new_item);
		if (str != NULL) {
			cJSON_AddStringToObject(new_item, minor, str);
		}
	}
	text = cJSON_Print(root);
	if (root) {
		cJSON_Delete(root);
		root = NULL;
	}
	config->config_contents = text;
	config->config_size = strlen(config->config_contents);
	cjson_config_write(config);
	config->valid = 1;
	pthread_mutex_unlock(&config->param_mutex);
	return 0;
}
int uninit_cjson_config(cjson_config_t *config)
{
	pthread_mutex_destroy(&config->param_mutex);
	if (config->config_contents != NULL) {
		free(config->config_contents);
		config->config_contents = NULL;
		config->config_size = 0;
	}
	config->valid = 0;
	memset(config, 0x00, sizeof(cjson_config_t));
	return 0;
}
