#include <tee_client_api.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define KEY_SSK_BYTE_LEN 32

#define SUNXI_UTILS_UUID \
{ 0x475f147a, 0x69c5, 0x11ea, \
	{ 0x10, 0xb8, 0x00, 0x50, 0x56, 0x97, 0x78, 0x83 } }

#define SUNXI_UTILS_CMD_EFUSE_BURN 2

#define EFUSE_DATA_SERIALIZE_OFFSET 96

typedef struct {
	char name[64];
	uint32_t len;
	uint32_t res;
	uint8_t *key_data;
} sunxi_efuse_key_info_t;

static const TEEC_UUID ta_UUID = SUNXI_UTILS_UUID;

void dump(uint8_t *buf, int ttl_len);

TEEC_Result burn_efuse(sunxi_efuse_key_info_t *efuse_info);

int efuse_info_serialize(uint8_t *out_buff, sunxi_efuse_key_info_t *efuse_info);
int efuse_write(char *key_file);

static void usage(void)
{
        printf("ssk_na - burn ssk to efuse \n\n");
        printf("usage:\n");
        printf("ssk_na -f [key_file]\n");
}

void dump(uint8_t *buf, int ttl_len)
{
	int len;
	for (len = 0; len < ttl_len; len++) {
		printf("0x%02x ", ((char *)buf)[len]);
		if (len % 8 == 7) {
			printf("\n");
		}
	}
	printf("\n");
}

int efuse_info_serialize(uint8_t *out_buff, sunxi_efuse_key_info_t *efuse_info)
{
	memcpy(out_buff, efuse_info, sizeof(sunxi_efuse_key_info_t));
	memcpy((uint8_t *)out_buff + EFUSE_DATA_SERIALIZE_OFFSET, efuse_info->key_data, efuse_info->len);
	return 0;
}

TEEC_Result burn_efuse(sunxi_efuse_key_info_t *efuse_info)
{
	TEEC_Context ctx;
	TEEC_Result teecErr;
	TEEC_Session teecSession;
	TEEC_Operation operation;

	printf("NA:Init Context!\n");
	teecErr = TEEC_InitializeContext(NULL, &ctx);
	if (teecErr != TEEC_SUCCESS)
		goto failInit;

	printf("NA:Open Session!\n");
	teecErr = TEEC_OpenSession(&ctx, &teecSession, &ta_UUID,
				   TEEC_LOGIN_PUBLIC, NULL, NULL, NULL);
	if (teecErr != TEEC_SUCCESS)
		goto failOpen;

	printf("NA:Allocate Memory!\n");
	TEEC_SharedMemory tee_params_p;
	tee_params_p.size  = 5000;
	tee_params_p.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
	if (TEEC_AllocateSharedMemory(&ctx, &tee_params_p) != TEEC_SUCCESS)
		goto failOpen;

	memset(&operation, 0, sizeof(TEEC_Operation));
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_NONE,
						TEEC_NONE, TEEC_NONE);
	operation.started = 1;

	operation.params[0].memref.parent = &tee_params_p;
	operation.params[0].memref.offset = 0;
	operation.params[0].memref.size   = 0;

	efuse_info_serialize(tee_params_p.buffer, efuse_info);
	//printf("raw input:\n");
	//dump(tee_params_p.buffer, EFUSE_DATA_SERIALIZE_OFFSET + efuse_info->len);

	printf("NA:Invoke Command\n");
	teecErr = TEEC_InvokeCommand(&teecSession, SUNXI_UTILS_CMD_EFUSE_BURN, &operation, NULL);

	TEEC_ReleaseSharedMemory(&tee_params_p);
failOpen:
	TEEC_FinalizeContext(&ctx);
failInit:
	printf("%s:finish with %x\n", __func__, teecErr);

	if (teecErr == TEEC_SUCCESS)
		return 0;
	else
		return teecErr;
}

int efuse_write(char *key_file)
{
	int ret = -1;
	int fd = 0;
	int read_len = 0;
	struct stat st;
	sunxi_efuse_key_info_t efuse_info;
	uint8_t key_data[128] = {0};

	if (stat(key_file, &st)) {
		printf("error: can not get key_file stat\n");
		return -1;
	}

	if (st.st_size > 128) {
		printf("error: key_file %s is too large!\n", key_file);
		return -1;
	}

	fd = open(key_file, O_RDONLY);
	if (fd == -1) {
		printf("error: can not open key_file %s\n", key_file);
		return -1;
	}

	read_len = read(fd, key_data, st.st_size);
	if (read_len == -1) {
		ret = -1;
		goto out;
	}

	memset(&efuse_info, 0, sizeof(sunxi_efuse_key_info_t));
	strcpy(efuse_info.name, "ssk");
	efuse_info.len = st.st_size;
	efuse_info.key_data = key_data;

	ret = burn_efuse(&efuse_info);

out:
	close(fd);

	return ret;
}

int main(int argc, char **argv)
{
	int ret = -1;
	int ch = 0;
	char *key_file = NULL;

	while ((ch = getopt(argc, argv, "f:")) != -1) {
		switch (ch) {
			case 'f':
				key_file = optarg;
				break;
			case '?':
			default:
				printf("Unknown option: %c\n", (char)optopt);
				usage();
				return -1;
				break;
		}
	}

	if (!key_file) {
		usage();
		return -1;
	}

	ret = efuse_write(key_file);

	return ret;
}
