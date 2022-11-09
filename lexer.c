#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define unsigned long long ull

// ����c�������б��������飬��ĸ��a��ʼ
static char *key[] = {"auto", "break", "case", "char", "const", "continue", "default", "do", "double",
                      "else", "enum", "extern", "float", "for", "goto", "if", "int", "long", "register",
                      "return", "short", "signed", "sizeof", "static", "struct", "switch", "typedef",
                      "union", "unsigned", "void", "volatile", "while"}; // length = 32

// ����c���������
static char *op[] = {"+", "-", "*", "/", "%", "++", "--", "=", "+=", "-=", "*=", "/=", "%=", "==", "!=",
                     ">", "<", ">=", "<=", "&", "|", "!", "&&", "||", "^", "~", "<<", ">>", "?", ":",
                     ",", ";", "(", ")", "[", "]", "{", "}", "\"", "\'"}; // length = 42

// ����c���������ַ�
static char *other[] = {"identifier", "number", "string", "character", "comment", "other"}; // length = 6

enum
{
    // ������
    _AUTO = 1,
    _BREAK,
    _CASE,
    _CHAR,
    _CONST,
    _CONTINUE,
    _DEFAULT,
    _DO,
    _DOUBLE,
    _ELSE,
    _ENUM,
    _EXTERN,
    _FLOAT,
    _FOR,
    _GOTO,
    _IF,
    _INT,
    _LONG,
    _REGISTER,
    _RETURN,
    _SHORT,
    _SIGNED,
    _SIZEOF,
    _STATIC,
    _STRUCT,
    _SWITCH,
    _TYPEDEF,
    _UNION,
    _UNSIGNED,
    _VOID,
    _VOLATILE,
    _WHILE,
    // �����
    PLUS,
    MINUS,
    STAR,
    DIV,
    MOD,
    PLUSPLUS,
    MINUSMINUS,
    ASSIGN,
    PLUSEQUAL,
    MINUSEQUAL,
    STAREQUAL,
    DIVEQUAL,
    MODEQUAL,
    EQUAL,
    NOTEQUAL,
    GREAT,
    LESS,
    GREATEQUAL,
    LESSEQUAL,
    AND,
    OR,
    NOT,
    ANDAND,
    OROR,
    BITXOR,
    BITNOT,
    LEFTMOVE,
    RIGHTMOVE,
    QUESTION,
    COLON,
    COMMA,
    SEMICOLON,
    LPARENT,
    RPARENT,
    LBRACKET,
    RBRACKET,
    LBRACE,
    RBRACE,
    DOUBLEQUOTE,
    SINGLEQUOTE,
    // ����
    INTCON,
    FLOATCON,
    CHARCON,
    STRCON,
    // ��ʶ��
    IDENFR,
    // �쳣
    _ERROR,
};

char sourceCode[1000000];

// ������ű������
int global_id_count = 0;
int curLine = 1;
int curCol = 1;
// ��¼����
char errorList[10000][100] = {""};
// ��¼�������
int errorCount = 0;
// ��¼��������ϣֵ
int global_id_hash[10000] = {0};

// �ж��Ƿ��Ǳ�����
int isKeyword(char s[])
{
    for (int i = 0; i < 32; i++)
        if (strcmp(s, key[i]) == 0)
            return i + 1;
    return 0;
}

// �ж��Ƿ�Ϊ��ĸ���»���
int isLetter(char c)
{
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
        return 1;
    return 0;
}

// �ж�ĳλ�Ƿ�Ϊ����
int isDigit(char c)
{
    if (c >= '0' && c <= '9')
        return 1;
    return 0;
}

int isIdentifier(char *c)
{
    if (isLetter(*c))
    {
        c++;
        while (isLetter(*c) || isDigit(*c))
            c++;
        if (*c == '\0')
            return 1;
    }
    return 0;
}

int isDecDigit(char *c)
{
    for (int i = 0; i < strlen(c); i++)
        if (!isDigit(c[i]))
            return 0;
    return 1;
}

// �ж��Ƿ�Ϊʮ����������
int isHexDigit(char *c)
{
    if (c[0] == '0' && !(c[1] == 'x' || c[1] == 'X'))
        return 0;

    for (int i = 2; i < strlen(c); i++)
        if (!isDigit(c[i]) && (c[i] < 'a' || c[i] > 'f') && (c[i] < 'A' || c[i] > 'F'))
            return 0;
    return 1;
}

// �ж��Ƿ�Ϊ�˽�������
int isOctDigit(char *c)
{
    if (c[0] != '0')
        return 0;

    for (int i = 1; i < strlen(c); i++)
        if (c[i] < '0' || c[i] > '7')
            return 0;
    return 1;
}
// ͨ��DFA�ж��Ƿ�Ϊ��ѧ������������
int isFloat(char *c)
{
    int state = 0;
    for (int i = 0; i < strlen(c); i++)
    {
        switch (state)
        {
        case 0:
            if (c[i] == '+' || c[i] == '-')
                state = 1;
            else if (isDigit(c[i]))
                state = 2;
            else
                return 0;
            break;
        case 1:
            if (isDigit(c[i]))
                state = 2;
            else
                return 0;
            break;
        case 2:
            if (isDigit(c[i]))
                state = 2;
            else if (c[i] == '.')
                state = 3;
            else if (c[i] == 'e' || c[i] == 'E')
                state = 5;
            else
                return 0;
            break;
        case 3:
            if (isDigit(c[i]))
                state = 4;
            else
                return 0;
            break;
        case 4:
            if (isDigit(c[i]))
                state = 4;
            else if (c[i] == 'e' || c[i] == 'E')
                state = 5;
            else
                return 0;
            break;
        case 5:
            if (c[i] == '+' || c[i] == '-')
                state = 6;
            else if (isDigit(c[i]))
                state = 7;
            else
                return 0;
            break;
        case 6:
            if (isDigit(c[i]))
                state = 7;
            else
                return 0;
            break;
        case 7:
            if (isDigit(c[i]))
                state = 7;
            else
                return 0;
            break;
        }
    }
    return 1;
}

int isOperatorSign(char c)
{
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '=' || c == '>' || c == '<' || c == '&' || c == '|' || c == '!' || c == '^' || c == '~' || c == '?' || c == ':' || c == ',' || c == ';' || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == '"' || c == '\'')
        return 1;
    return 0;
}

int isOperator(char *c)
{
    for (int i = 0; i < 42; i++)
        if (strcmp(c, op[i]) == 0)
            return i + 33;
    return 0;
}

// Ԥ���룬��������ע�͡��ո��Ʊ��
int filterStopWord(char *r, int totLen)
{
    char rawCode[100000] = "";
    int pos = 0;  // ��¼��ǰ�ַ�λ��
    int line = 0; // �к�
    int col = 0;  // �к�
    for (int i = 0; i < totLen; i++)
    {
        if (r[i] == '/' && r[i + 1] == '/')
        {
            while (r[i] != '\n')
                i++;
            line++;
        }
        if (r[i] == '#')
        {
            while (r[i] != '\n')
                i++;
            line++;
            i --;
        }
        else if (r[i] == '\n')
        {
            line++;
            col = 1;
            rawCode[pos++] = r[i];
        }
        else if (r[i] == '\r' || r[i] == '\t')
            col++;
        else if (r[i] == '/' && r[i + 1] == '*')
        {
            while (!(r[i] == '*' && r[i + 1] == '/'))
            {
                i++;
                int originCol = col;
                int sourceLine = line;
                if (r[i] == '\n')
                {
                    sourceLine++;
                    rawCode[pos++] = r[i];
                }
                // �ж��Ƿ���δ�պϵ�ע��
                if (i == totLen - 1)
                {
                    // ��δ�պϵ�ע�ͺ����кš��кż�¼��������
                    char exception[100] = "ERROR: line ";
                    // ���к�ƴ����������
                    char temp[10];
                    sprintf(temp, "%d", sourceLine);
                    strcat(exception, temp);
                    strcat(exception, " col ");
                    // ���к�ƴ����������
                    sprintf(temp, "%d", originCol);
                    strcat(exception, temp);
                    strcat(exception, ": comments should be closed");
                    // ��������������󼯺���
                    strcpy(errorList[errorCount], exception);
                    printf("%s", exception);
                    errorCount++;
                    rawCode[i++] = '\0';
                    exit(0);
                }
            }
            i++;
        }
        else if (r[i] == ' ' && r[i + 1] == ' ')
        {
            col++;
        }
        else
        {
            rawCode[pos++] = r[i];
            col++;
        }
    }
    rawCode[pos] = '\0';
    strcpy(r, rawCode);
    return 1;
}

// �����ӳ���tokenlize
void tokenize(int *codePos, char *sourceCode, char *token, int *tokenType)
{
    int tokenLen = 0;
    char start = sourceCode[*codePos];

    int col = curCol;
    int line = curLine;

    int tmpCol = col;
    int tmpLine = line;

    strcpy(token, "");
    if (start == '\n')
    {
        line++;
        col = 1;
        *tokenType = -2;
        *codePos += 1;
    }
    else if (isLetter(start))
    {
        while (isLetter(sourceCode[*codePos]) || isDigit(sourceCode[*codePos]))
        {
            token[tokenLen++] = sourceCode[(*codePos)++];
            col++;
        }
        token[tokenLen] = '\0';
        int flag = isKeyword(token);
        *tokenType = flag ? flag : IDENFR;
    }
    else if (isDigit(start))
    {
        while (isDigit(sourceCode[*codePos]) || isLetter(sourceCode[*codePos]))
        {
            token[tokenLen++] = sourceCode[(*codePos)++];
            col++;
        }
        token[tokenLen] = '\0';
        // �ж��Ƿ�Ϊ16������
        if (isHexDigit(token))
            *tokenType = INTCON;
        else if (isOctDigit(token))
            *tokenType = INTCON;
        else if (isDecDigit(token))
            *tokenType = INTCON;
        else if (isFloat(token))
            *tokenType = FLOATCON;
        else
        {
            errorCount++;
            *tokenType = _ERROR;
            char exception[100] = "ERROR: line ";
            char temp[10];
            sprintf(temp, "%d", tmpLine);
            strcat(exception, temp);
            strcat(exception, " col ");
            sprintf(temp, "%d", tmpCol);
            strcat(exception, temp);
            strcat(exception, ": illegal number: ");
            strcat(exception, token);
            strcpy(errorList[errorCount], exception);
            printf("%s\n", exception);
            return;
        }
    }
    else if (start == ' ')
    {
        *codePos += 1;
        col++;
        *tokenType = -1;
    }
    else if (start == '\'')
    {
        if (sourceCode[(*codePos) + 1] == '\\' && sourceCode[(*codePos) + 3] == '\'')
        {
            token[tokenLen++] = sourceCode[(*codePos)++];
            token[tokenLen++] = sourceCode[(*codePos)++];
            token[tokenLen++] = sourceCode[(*codePos)++];
            token[tokenLen++] = sourceCode[(*codePos)++];
            token[tokenLen] = '\0';
            *tokenType = CHARCON;
            col += 4;
            return;
        }

        token[tokenLen++] = sourceCode[(*codePos)++];
        token[tokenLen++] = sourceCode[(*codePos)++];
        token[tokenLen++] = sourceCode[(*codePos)++];
        token[tokenLen] = '\0';
        col += 3;
        if (token[2] != '\'')
        {
            errorCount++;
            *tokenType = _ERROR;
            char exception[100] = "ERROR: line ";
            char temp[10];
            sprintf(temp, "%d", tmpLine);
            strcat(exception, temp);
            strcat(exception, " col ");
            sprintf(temp, "%d", tmpCol);
            strcat(exception, temp);
            strcat(exception, ": char should be enclosed by single quotes");
            strcpy(errorList[errorCount], exception);
            // printf("%s\n", exception);
            return;
        }
        *tokenType = CHARCON;
    }
    else if (start == '\"')
    {
        token[tokenLen++] = sourceCode[(*codePos)++];
        col++;
        while (sourceCode[*codePos] != '\"')
        {
            if (sourceCode[*codePos] == '\\')
            {
                token[tokenLen++] = sourceCode[(*codePos)++];
                col++;
            }

            token[tokenLen++] = sourceCode[(*codePos)++];
            col++;
            // ���ַ���δ�պ�
            if (*codePos >= strlen(sourceCode))
            {
                errorCount++;
                *tokenType = _ERROR;
                char exception[100] = "ERROR: line ";
                char temp[10];
                sprintf(temp, "%d", tmpLine);
                strcat(exception, temp);
                strcat(exception, " col ");
                sprintf(temp, "%d", tmpCol);
                strcat(exception, temp);
                strcat(exception, ": string should be closed with double quotes");
                strcpy(errorList[errorCount], exception);
                printf("%s\n", exception);
                return;
            }
        }
        token[tokenLen++] = sourceCode[(*codePos)++];
        token[tokenLen] = '\0';
        col++;
        *tokenType = STRCON;
    }
    else if (isOperatorSign(start))
    {
        // ����λ�Ƿ��ţ�����һλҲ�Ƿ���
        if (sourceCode[*codePos + 1] == '+' && sourceCode[*codePos + 1] == '-' && sourceCode[*codePos + 1] == '=' && sourceCode[*codePos + 1] == '>' && sourceCode[*codePos + 1] == '<' && sourceCode[*codePos + 1] == '&' && sourceCode[*codePos + 1] == '|' && sourceCode[*codePos + 1] == '\'' && sourceCode[*codePos + 1] == '\"')
        {
            token[tokenLen++] = sourceCode[(*codePos)++];
            token[tokenLen++] = sourceCode[(*codePos)++];
            col += 2;
        }
        else
        {
            token[tokenLen++] = sourceCode[(*codePos)++];
            col++;
        }
        token[tokenLen] = '\0';
        int flag = isOperator(token);
        if (flag)
        {
            *tokenType = flag;
        }
        else
        {
            errorCount++;
            *tokenType = _ERROR;
            char exception[100] = "ERROR: line ";
            char temp[10];
            sprintf(temp, "%d", tmpLine);
            strcat(exception, temp);
            strcat(exception, " col ");
            sprintf(temp, "%d", tmpCol);
            strcat(exception, temp);
            strcat(exception, ": ilegal operator: ");
            strcat(exception, token);
            strcpy(errorList[errorCount], exception);
            printf("%s\n", exception);
            return;
        }
    }
    else if (start == EOF || start == '\0')
    {
        *tokenType = 0;
    }
    else
    {
        errorCount++;
        *tokenType = _ERROR;
        char exception[100] = "ERROR: line ";
        char temp[10];
        sprintf(temp, "%d", tmpLine);
        strcat(exception, temp);
        strcat(exception, " col ");
        sprintf(temp, "%d", tmpCol);
        strcat(exception, temp);
        strcat(exception, ": illegal word: ");
        char temp2[2];
        temp2[0] = start, temp2[1] = '\0';
        strcat(exception, temp2);
        strcpy(errorList[errorCount], exception);
        printf("%s\n", exception);
        return;
    }
    // �������к�
    curLine = line;
    curCol = col;
}

// �жϸô��Ƿ�Ϊָ���Ĺؼ��֣������������һ��token�����򱨴��˳�
void assert(int wanted_tk, int *codePos, char *sourceCode, char *token, int *tokenType)
{
    if (*tokenType != wanted_tk)
    {
        // printf("%d  %s", *tokenType, token);
        printf("line %lld, col %lld: expect token: %d, get: %s\n", curLine, curCol, wanted_tk, token);
        exit(-1);
    }
    // ��ֹ���ո�
    do
    {
        tokenize(codePos, sourceCode, token, tokenType);
    } while (*tokenType <= 0);
}

// �жϱ����Ƿ��ظ�
void check_new_id(char *token)
{
    int hash = 0, P = 1;
    // �����ϣֵ
    for (int i = 0; i < strlen(token); i++)
    {
        hash += (token[i] * P) % 1610612741;
        P = P * 131 % 1610612741;
    }
    // �ж��Ƿ��ظ�
    for (int i = 0; i < global_id_count; i++)
    {
        if (global_id_hash[i] == hash)
        {
            printf("line %lld, col %lld: variable %s has been declared\n", curLine, curCol, token);
            exit(-1);
        }
    }
    // ��ӵ�ȫ�ֱ�����
    global_id_hash[global_id_count++] = hash;
}

// ����ö��enum���ķ�
void parse_enum(int *codePos, char *sourceCode, char *token, int *tokenType)
{
    int enum_val = 0;
    while (*tokenType != RBRACE)
    {
        check_new_id(token);
        assert(IDENFR, codePos, sourceCode, token, tokenType);
        if (*tokenType == ASSIGN)
        {
            assert(ASSIGN, codePos, sourceCode, token, tokenType);
            assert(INTCON, codePos, sourceCode, token, tokenType);
        }
        if (*tokenType == COMMA)
            // ��ֹ���ո�
            do
            {
                tokenize(codePos, sourceCode, token, tokenType);
            } while (*tokenType <= 0);
    }
}

void parser(char *sourceCode)
{
    int codePos = 0;
    int tokenType = 1;
    char token[100];
    int line = 1;
    int col = 1;
    while (tokenType != 0)
    {   
        // ��ֹ���ո�
        do
        {
            tokenize(&codePos, sourceCode, token, &tokenType);
        } while (tokenType < 0);
        // tokenize(&codePos, sourceCode, token, &tokenType);
        if (tokenType == _ENUM)
        {
            // ��ֹ���ո�
            do
            {
                tokenize(&codePos, sourceCode, token, &tokenType);
            } while (tokenType < 0); // ȡ��һ�����ַ�
            if (tokenType != LBRACE)
                assert(IDENFR, &codePos, sourceCode, token, &tokenType);

            assert(LBRACE, &codePos, sourceCode, token, &tokenType);
            parse_enum(&codePos, sourceCode, token, &tokenType);
            assert(RBRACE, &codePos, sourceCode, token, &tokenType);
        }
    }
}

int main()
{
    // ��ȡԴ����
    FILE *fp = fopen("lexer.c", "r");
    if (fp == NULL)
    {
        printf("ERROR: file not found");
        return 0;
    }
    char ch;
    int codeLen = 0;
    while ((ch = fgetc(fp)) != EOF)
        sourceCode[codeLen++] = ch;
    // printf("%d", codeLen);
    int flag = filterStopWord(sourceCode, strlen(sourceCode));
    if (!flag)
    {
        printf("Ԥ����ʧ�ܣ�");
        return 0;
    }
    // printf("%s", sourceCode);
    parser(sourceCode);
    // char token[100];
    // int tokenType;
    // int codePos = 0;
    // while (codePos < strlen(sourceCode))
    // {
    //     tokenize(&codePos, sourceCode, token, &tokenType);
    //     if (tokenType)
    //     {
    //         global_id_count++;
    //         if (tokenType <= 29)
    //             printf("Reserved Word: %s\n", token);
    //         else if (tokenType <= 74)
    //             printf("Operator: %s\n", token);
    //         else if (tokenType <= 78)
    //             printf("Literal: %s\n", token);
    //         else if (tokenType == 79)
    //             printf("Identifier: %s\n", token);
    //         else
    //             break;
    //     }
    // }
    return 0;
}
