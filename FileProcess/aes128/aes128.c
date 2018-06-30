#include "aes128.h"
void subBytes(int[4][4], int);
void shiftRows(int[4][4], int);
void mixColumns(int[4][4], int);
void addRoundKey(int[4][4], int[4][4]);
int aes_multiple(int, int);
void keyExpansion(int key[4][4], int w[11][4][4]);
void aes(char*, char*, char*, int);


int test() {
    int method = 0;//1表示加密， 0表示解密
                   //待加密/解密文件存放路径
    char * source_path = "G:\\aes2.txt";
    // 加密/解密后文件存放路径
    char *des_path = "G:\\aes3.txt";
    // 32位16进制密钥
    char * password = "0f1571c947d9e8590cb7add6af7f6798";
    aes(source_path, des_path, password, method);
    printf("success!!!!!!!!!!");
    return 0;
}

void aes(char* source_path, char* des_path, char* password, int method) {
    //将密钥转换成4*4数组
    int p[4][4];
    for (int m = 0; m < 4; ++m) {
        for (int i = 0; i < 4; ++i) {
            int indx = 4 * i + m;
            p[i][m] = 16 * c2i(password[indx]) + c2i(password[indx + 1]);
        }
    }

    FILE *file = fopen(source_path, "r"); //获取文件的指针
    fseek(file, 0, SEEK_END); //移动文件的指针到文件结尾
    int len = ftell(file); //获取文件的长度
    rewind(file); //将文件指针移动回文件开始
                  // 如果文件长度不是128位（16字节）的整数倍，则补齐
                  //int size = len;
                  //if (len % 16 != 0) {
                  //    size = (len / 16 + 1) * 16;
                  //}
                  //unsigned char content[size];
                  ////读取文件内容赋值给content
                  //fread(content, 1, len, file);
                  //for (int j = len; j < size; ++j) {
                  //    content[j] = 0;
                  //}
                  //fclose(file);
                  ////存储结果
                  //unsigned char encry[size];
                  ////将文件转换成16字节的int型数组加密、解密
                  //for (int i = 0; i < size / 16; ++i) {

                  //    int content_to_int[4][4];
                  //    for (int j = 0; j < 4; ++j) {
                  //        for (int k = 0; k < 4; ++k) {
                  //            content_to_int[j][k] = content[j * 4 + k + 16 * i];
                  //        }
                  //    }
                  //    aes_detail(content_to_int, p, method);
                  //    for (int j = 0; j < 4; ++j) {
                  //        for (int k = 0; k < 4; ++k) {
                  //            encry[j * 4 + k + 16 * i] = content_to_int[j][k];
                  //        }
                  //    }
                  //}
                  //FILE *file1 = fopen(des_path, "w");
                  //fwrite(encry, size, 1, file1);
                  //fflush(file1);
                  //fclose(file1);
}

void aes_detail(int content[4][4], int password[4][4], int encode) {
    int p[11][4][4];
    keyExpansion(password, p);

    if (encode) {
        addRoundKey(content, p[0]);
        for (int i = 1; i <= 10; ++i) {
            subBytes(content, encode);
            shiftRows(content, encode);
            if (i != 10) {
                mixColumns(content, encode);
            }

            addRoundKey(content, p[i]);
        }
    } else {
        addRoundKey(content, p[10]);
        for (int i = 9; i >= 0; --i) {
            shiftRows(content, encode);
            subBytes(content, encode);
            addRoundKey(content, p[i]);
            if (i != 0) {
                mixColumns(content, encode);
            }
        }
    }
}

void subBytes(int a[4][4], int encode) {
    // encode 为1 代表字节替代，为0代表逆向字节替代
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            int temp = a[i][j];
            int row = temp / 16;
            int column = temp % 16;
            if (encode)
                a[i][j] = S_BOX[row][column];
            else
                a[i][j] = INVERSE_S_BOX[row][column];
        }
    }
}

void shiftRows(int a[4][4], int encode) {
    //encode 为1代表行移位，为0代表逆向行移位
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < i; ++j) {
            if (encode) {
                int temp = a[i][0];
                a[i][0] = a[i][1];
                a[i][1] = a[i][2];
                a[i][2] = a[i][3];
                a[i][3] = temp;
            } else {
                int temp = a[i][3];
                a[i][3] = a[i][2];
                a[i][2] = a[i][1];
                a[i][1] = a[i][0];
                a[i][0] = temp;
            }
        }
    }
}

void mixColumns(int a[4][4], int encode) {
    //encode 为1代表列混淆，为0代表逆向列混淆
    for (int i = 0; i < 4; ++i) {
        int temp0 = a[0][i];
        int temp1 = a[1][i];
        int temp2 = a[2][i];
        int temp3 = a[3][i];
        if (encode) {
            a[0][i] = aes_multiple(temp0, 2) ^ aes_multiple(temp1, 3) ^ temp2 ^ temp3;
            a[1][i] = temp0 ^ (aes_multiple(temp1, 2)) ^ (temp2 ^ aes_multiple(temp2, 2)) ^ temp3;
            a[2][i] = temp0 ^ temp1 ^ (aes_multiple(temp2, 2)) ^ (temp3 ^ aes_multiple(temp3, 2));
            a[3][i] = temp0 ^ (aes_multiple(temp0, 2)) ^ temp1 ^ temp2 ^ aes_multiple(temp3, 2);
        } else {
            a[0][i] = aes_multiple(temp0, 14) ^ aes_multiple(temp1, 11) ^ aes_multiple(temp2, 13) ^ aes_multiple(temp3, 9);
            a[1][i] = aes_multiple(temp0, 9) ^ aes_multiple(temp1, 14) ^ aes_multiple(temp2, 11) ^ aes_multiple(temp3, 13);
            a[2][i] = aes_multiple(temp0, 13) ^ aes_multiple(temp1, 9) ^ aes_multiple(temp2, 14) ^ aes_multiple(temp3, 11);
            a[3][i] = aes_multiple(temp0, 11) ^ aes_multiple(temp1, 13) ^ aes_multiple(temp2, 9) ^ aes_multiple(temp3, 14);
        }
    }
}

void addRoundKey(int a[4][4], int k[4][4]) {
    // 由于用w[11][4][4]表示W[44]导致行列转置，所以在进行异或操作的时候应该是a[i，j] 异或 k[j,i]
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            a[i][j] = a[i][j] ^ k[j][i];
        }
    }
}

//AES乘法计算
int aes_multiple(int a, int le) {
    int thr = le & 0x8;
    int sec = le & 0x4;
    int fir = le & 0x2;
    int fir_mod = le % 2;
    int result = 0;
    if (thr) {
        int b = a;
        for (int i = 1; i <= 3; ++i) {
            b = b << 1;
            if (b >= 256)
                b = b ^ 0x11b;
        }
        b = b % 256;
        result = result ^ b;
    }
    if (sec) {
        int b = a;
        for (int i = 1; i <= 2; ++i) {
            b = b << 1;
            if (b >= 256)
                b = b ^ 0x11b;
        }
        b = b % 256;
        result = result ^ b;
    }
    if (fir) {
        int b = a << 1;
        if (b >= 256)
            b = b ^ 0x11b;
        b = b % 256;
        result = result ^ b;
    }
    if (fir_mod)
        result = result ^ a;
    return  result;
}

void keyExpansion(int key[4][4], int w[11][4][4]) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            w[0][i][j] = key[j][i];
        }
    }
    for (int i = 1; i < 11; ++i) {
        for (int j = 0; j < 4; ++j) {
            int temp[4];
            if (j == 0) {
                temp[0] = w[i - 1][3][1];
                temp[1] = w[i - 1][3][2];
                temp[2] = w[i - 1][3][3];
                temp[3] = w[i - 1][3][0];
                for (int k = 0; k < 4; ++k) {
                    int m = temp[k];
                    int row = m / 16;
                    int column = m % 16;
                    temp[k] = S_BOX[row][column];
                    if (k == 0) {
                        temp[k] = temp[k] ^ RC[i - 1];
                    }
                }
            } else {
                temp[0] = w[i][j - 1][0];
                temp[1] = w[i][j - 1][1];
                temp[2] = w[i][j - 1][2];
                temp[3] = w[i][j - 1][3];
            }
            for (int l = 0; l < 4; ++l) {

                w[i][j][l] = w[i - 1][j][l] ^ temp[l];
            }

        }
    }

}

//将字符转换为数值
int c2i(char ch) {
    // 如果是数字，则用数字的ASCII码减去48, 如果ch = '2' ,则 '2' - 48 = 2
    if (isdigit(ch))
        return ch - 48;

    // 如果是字母，但不是A~F,a~f则返回
    if (ch < 'A' || (ch > 'F' && ch < 'a') || ch > 'z')
        return -1;

    // 如果是大写字母，则用数字的ASCII码减去55, 如果ch = 'A' ,则 'A' - 55 = 10
    // 如果是小写字母，则用数字的ASCII码减去87, 如果ch = 'a' ,则 'a' - 87 = 10
    if (isalpha(ch))
        return isupper(ch) ? ch - 55 : ch - 87;

    return -1;
}