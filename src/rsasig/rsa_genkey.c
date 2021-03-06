#include <stdio.h>
#include <string.h>
#include "rsasig/rsa_genkey.h"
#include "rsasig/base64.h"
#include "openssl/rsa.h"
#include "openssl/pem.h"
#include "openssl/evp.h"

int pubkey_pemtoder(char *pem, unsigned char **der)
{
    unsigned char tempkey[4096];
    int pem_len = strlen(pem);
    int index = 0;
    int der_len = 0;

    for (int i = 0;i < pem_len; ++i){
        if (pem[i] == '\n'){
            continue;
        }
        tempkey[index++] = pem[i];
    }
    tempkey[index-24] = '\0';
    der_len = base64_decode(tempkey+26, der);

    *der += 24;
    return der_len - 24;
}

int privkey_pemtoder(char *pem, unsigned char **der)
{
    unsigned char tempkey[4096] = {0};
    int pem_len = strlen(pem);
    int index = 0;
    int der_len = 0;

    for (int i = 0;i < pem_len; ++i){
        if (pem[i] == '\n'){
            continue;
        }
        tempkey[index++] = pem[i];
    }
    tempkey[index-25] = '\0';
    der_len = base64_decode(tempkey+27, der);
    *der += 26;
    return der_len - 26;
}

/*-------------------------------------------------------
 密钥生成过程
-------------------------------------------------------*/
int Generate_RSA_Keys(const int g_nBits, char *pubkey , char *privkey)
{
	/*---------------------------------------------------
	 *说明：
	 *g_nBits表示加密长度，意味着最多可以编解码g_nBits/8-11=117个字节，
	 *RSA_F4即65537，为公钥指数，一般情况下使用RSA_F4即可，其它两个参数
	 *可以设置为NULL，为了兼容Crypto++密码库生成的密钥，公钥使用传统PEM
	 *公钥格式进行存取，私钥则采用PKCS#8非加密私钥格式进行存取
	---------------------------------------------------*/	
    
	/*对RSA结构体和EVP_KEY结构体进行初始化*/
	RSA *pRsa = RSA_new();
	EVP_PKEY *eRsa = EVP_PKEY_new();

	/*生成密钥对并保存在RSA结构体中*/
	pRsa = RSA_generate_key(g_nBits,RSA_F4,NULL,NULL);
	if (pRsa == NULL)
	{
		printf("Rsa_generate_key error\n");
		return -1;
	}

	/*建立一个保存公钥的可读/写内存BIO*/
	BIO *pub = BIO_new(BIO_s_mem());

	/*从RSA结构体中提取公钥到BIO中*/
	PEM_write_bio_RSA_PUBKEY(pub,pRsa);

	/*BIO中的公钥保存到char数组中*/
	int pub_len = BIO_read(pub,pubkey,4096);
	if(pub_len == 0){
		printf("Generate Publickey error\n");
		return -2;
	}
    /*至此得到的是pem格式的公钥 */
	pubkey[pub_len] = '\0';

	/*释放存放公钥的BIO内存*/
	BIO_free(pub);

	/*建立一个保存私钥的可读/写内存BIO*/
	BIO *pri = BIO_new(BIO_s_mem());
	
	/*用EVP_PKEY结构体替换RSA结构体*/
	EVP_PKEY_assign_RSA(eRsa,pRsa);

	/*从EVP_PKEY结构体中提取私钥到BIO中*/
	PEM_write_bio_PKCS8PrivateKey(pri,eRsa,NULL,NULL,0,NULL,NULL);
	
	/*将BIO中的公钥保存到char数组中*/
	int pri_len = BIO_read(pri,privkey,4096);
	if(pri_len == 0){
		/*printf("Generate Privatekey error\n");*/
		return -3;
	}
	privkey[pri_len] = '\0';

	/*释放存放私钥的BIO内存和EVP_PKEY结构体*/
	BIO_free(pri);
	EVP_PKEY_free(eRsa);		/*EVP_PKEY结构体已经替换了RSA结构体，无需再释放RSA结构体*/
	return 0;
}

/*-------------------------------------------------------
 对公钥进行PEM格式化
-------------------------------------------------------*/
void PubKeyPEMFormat(char *pubkey,int nPublicKeyLen)
{
	char format_pubkey[4096];
	char pub_tem[4096];
	int index = 0,publength = 0;
	
	/*对字符串数组进行初始化*/
	memset(format_pubkey, 0, sizeof(format_pubkey));
	memset(pub_tem, 0, sizeof(pub_tem));
	
	char *pub_begin = "-----BEGIN PUBLIC KEY-----\n";
	char *pub_end = "-----END PUBLIC KEY-----\n";
	char *check = strstr(pubkey,pub_begin);
	if(check)
	{
		return;
	}
	else
	{		
		memcpy(format_pubkey,pub_begin,27);
		for(index = 0; index < nPublicKeyLen; index += 64)
		{			
			memcpy(pub_tem,pubkey+index,64);
			strcat(format_pubkey,pub_tem);
			publength = strlen(format_pubkey);
			format_pubkey[publength] = '\n';
			memset(pub_tem, 0, sizeof(pub_tem));
		}
		strcat(format_pubkey,pub_end);
		memcpy(pubkey,format_pubkey,strlen(format_pubkey));
	}
}


/*-------------------------------------------------------
 对私钥进行PEM格式化
-------------------------------------------------------*/
void PrivKeyPEMFormat(char *privkey)
{
	char format_privkey[4096];
	char priv_tem[4096];
	int index = 0,privlength = 0,nPrivateKeyLen = 0;
	
	/*对字符串数组进行初始化*/
	memset(format_privkey, 0, sizeof(format_privkey));
	memset(priv_tem, 0, sizeof(priv_tem));
	
	char *priv_begin = "-----BEGIN PRIVATE KEY-----\n";
	char *priv_end = "-----END PRIVATE KEY-----\n";
	char *check = strstr(privkey, priv_begin); 
	if(check)
	{
		return;
	}
	else
	{
		nPrivateKeyLen = strlen(privkey); 
		memcpy(format_privkey,priv_begin,28);
		for(index = 0; index < nPrivateKeyLen; index += 64)
		{			
			memcpy(priv_tem,privkey+index,64);
			strcat(format_privkey,priv_tem);
			privlength = strlen(format_privkey);
			format_privkey[privlength] = '\n';
			memset(priv_tem, 0, sizeof(priv_tem));
		}
		strcat(format_privkey,priv_end);
		memcpy(privkey,format_privkey,strlen(format_privkey));
	}
}