#pragma warning(disable:4996)

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MAX 1000
#define v_len 20

//token类型
enum Token_Tag 
{
    START, INT, FLOAT, SIGN, VARIABLE, TYPE, EQUAL, WRITE, END, ALLEND
};

//符号类型
enum Sign 
{
    PLUS = '+', MINUS = '-', MUL = '*', DIV = '/', LP = '(', RP = ')'
};

typedef struct _Token
{
    int Token_tag;
    union Value
    {
        int sign;
        int i_val;
        float f_val;
        char v_val[v_len];//用于存放变量
    } value;

} Token;

//变量类型
typedef struct _Id
{
    int id_type_float;
    char id_name[v_len];
    int id_num_bool;
    union Id_num
    {
        int id_int;
        float id_float;
    }id_num;
}Id;
//堆栈
typedef struct _Stack
{
    Token* top;
    Token* base;
    int size;
}Stack;

//文件读取与写入函数//
int file_read(char[], char[]);  //文件读操作,返回值为1表示读取成功，否则为0
int file_write(Id pro_res, char f_path[]);


//栈相关
void init_stack(Stack* stack);
void free_stack(Stack* stack);
void push(Stack* stack, Token t);
Token* pop(Stack* stack);
Token* top(Stack* stack);
void inverse(Stack* stack);

int scan_token(char pro[], int pro_len, int offset, Token* current);//扫描函数，扫描下一字符
void error(const char* message);//报错函数
int cal(char pro[], int offset, int pro_len, Token* cal_result);//算术表达式的计算
void cal_token(Token* t1, Token* t2, int sign, Token* result);//二元计算
int get_op_level(int op);//判断符号优先级

int line = 1;//扫描所在行数
int id_sum = 0;//存入变量总数
int pro_len;//输入程序的字符数目
Id pro_id[v_len];//程序允许的最多变量不得超过20个
int pro_write = 0;//判断是否有输出语句

int main()
{
    char f_path[200] = "D:\\";//存储读取文件路径
    char pro[MAX] = { 0 };//存入程序的字符串
    if (!file_read(pro, f_path))
    {
        return -1;
    }
    pro_len = strlen(pro);


    Token* current;//正在扫描的token
    Id tem_id;
    int tem_id_num = -1;//查询存储变量的数组下标

    current = (Token*)malloc(sizeof(Token));
    int index = 0;//程序字符串的偏移量

    current->Token_tag = START;

    do
    {
        index = scan_token(pro, pro_len, index, current);
        switch (current->Token_tag)
        {
        case START:
        {
            error("error: find no token");
            break;
        }
        case TYPE:
        {
            if (strcmp(current->value.v_val, "float") == 0)
            {
                tem_id.id_type_float = 1;
            }
            else
            {
                tem_id.id_type_float = 0;
            }

            index = scan_token(pro, pro_len, index, current);
            if (current->Token_tag != VARIABLE)
            {
                error("error: no variable");
            }
            for (int i = 0; i < id_sum; i++)
            {
                if (strcmp(current->value.v_val, pro_id[i].id_name) == 0)
                    error("error: conflicting declaration");
            }
            strcpy(pro_id[id_sum].id_name, current->value.v_val);
            pro_id[id_sum].id_type_float = tem_id.id_type_float;
            pro_id[id_sum].id_num_bool = 0;
            id_sum++;
            if (id_sum >= v_len)
            {
                error("error: variable overflow");
            }
            index = scan_token(pro, pro_len, index, current);
            if (current->Token_tag != END && current->Token_tag != ALLEND)
            {
                error("error : no ; or .");
            }
            break;
        }

        case VARIABLE:
        {
            int j = 0;
            for (int i = 0; i < id_sum; i++)
            {
                if (strcmp(current->value.v_val, pro_id[i].id_name) == 0)
                {
                    tem_id_num = i;
                    j = 1;
                    break;
                }
            }
            if (j == 0)
            {
                error("error: undefined identifier");
            }

            index = scan_token(pro, pro_len, index, current);
            if (current->Token_tag != EQUAL)
            {
                error("error: no = ");
            }
            index = cal(pro, index, pro_len, current);
            if (index == -1)//表示返回值为ALLEND结束计算
            {
                {
                    printf("\n\n计算完成\n");
                    if (pro_write == 1)
                    {
                        printf("输出结果存储在读入文件末尾，请注意检验\n");
                        printf("本次输出所有浮点型均保留两位小数，采取四舍五入进制\n");
                        printf("如果程序出错则无法查询到计算结果\n");
                    }
                    else
                    {
                        printf("无输出结果指令，无法查询计算结果\n");
                        printf("如果程序出错同样无法查询到计算结果\n");
                    }

                    system("PAUSE");
                    return 0;
                }
            }
            if (current->Token_tag == INT)
            {
                if (pro_id[tem_id_num].id_type_float == 1)//int->float
                {
                    error("error: int cannot be converted into float");
                }
                else//int -> int
                {
                    pro_id[tem_id_num].id_num.id_int = current->value.i_val;
                }
            }
            else
            {
                if (pro_id[tem_id_num].id_type_float == 1)//float->float
                {
                    pro_id[tem_id_num].id_num.id_float = current->value.f_val;
                }
                else//float->int
                {
                    pro_id[tem_id_num].id_num.id_int = (int)current->value.f_val;
                }
            }
            pro_id[tem_id_num].id_num_bool = 1;
            //if (pro[index - 1] != ';')
            //{
            //   error("syntax error: no ;");
            //}
            break;
        }
        case WRITE:
        {
            if (pro[index] != '(')
            {
                error("error: no (");
            }
            index++;
            index = scan_token(pro, pro_len, index, current);
            if (current->Token_tag != VARIABLE)
            {
                error("error: no variable");
            }
            int j = 0;
            for (int i = 0; i < id_sum; i++)
            {
                if (strcmp(current->value.v_val, pro_id[i].id_name) == 0)
                {
                    tem_id_num = i;
                    j = 1;
                    break;
                }
            }
            if (j == 0)
            {
                error("error: undefined identifier");
            }
            if (pro[index] != ')')
            {
                error("error: no )");
            }
            index++;
            index = scan_token(pro, pro_len, index, current);
            if (current->Token_tag != END && current->Token_tag != ALLEND)
            {
                error("error: no ; or .");
            }
            
            if (pro_id[tem_id_num].id_num_bool == 0)
            {
                error("error: variable is not assigned \n");
            }
            if (pro_id[tem_id_num].id_type_float == 1)
            {
                printf("%.2f\n", pro_id[tem_id_num].id_num.id_float);
                pro_write = file_write(pro_id[tem_id_num], f_path);
            }
            else
            {
                printf("%d\n", pro_id[tem_id_num].id_num.id_int);
                pro_write = file_write(pro_id[tem_id_num], f_path);
            }
            if (current->Token_tag == ALLEND)
            {
                printf("\n\n计算完成\n");
                if (pro_write == 1)
                {
                    printf("输出结果存储在读入文件末尾，请注意检验\n");
                    printf("本次输出所有浮点型均保留两位小数，采取四舍五入进制\n");
                    printf("如果程序出错则无法查询到计算结果\n");
                }
                else
                {
                    printf("无输出结果指令，无法查询计算结果\n");
                    printf("如果程序出错同样无法查询到计算结果\n");
                }

                system("PAUSE");
                return 0;
            }
            break;
        }
        case END:
        {
            break;
        }
        case ALLEND:
        {
            printf("\n\n计算完成\n");
            if (pro_write == 1)
            {
                printf("输出结果存储在读入文件末尾，请注意检验\n");
                printf("本次输出所有浮点型均保留两位小数，采取四舍五入进制\n");
                printf("如果程序出错则无法查询到计算结果\n");
            }
            else
            {
                printf("无输出结果指令，无法查询计算结果\n");
                printf("如果程序出错同样无法查询到计算结果\n");
            }

            system("PAUSE");
            return 0;
        }
        default:
        {
            error("error: syntax error");
            break;
        }
        }

    } while (index < pro_len);

    system("PAUSE");
    return 0;
}

//文件读取和写入
int file_read(char pro[], char f_path[])
{
    printf("请确定需计算的内容已存为txt格式，并放至D盘目录下\n");
    printf("请输入文件名称（如【pro.txt】可输入【pro.txt】\n");
    printf("请输入：\n");

    char f_name[100];

    scanf("%s", &f_name);
    strcat(f_path, f_name);

    FILE* fp;
    int err = 0;
    if (fp = fopen(f_path, "r"))
    {
        err = 1;
        int i = 0;
        char j;
        do
        {
            j = fgetc(fp);
            pro[i] = j;
            i++;
        } while (j != EOF);
        printf("读取成功，读取内容为：\n");
        printf("%s\n\n", pro);
        printf("正在计算……\n");
        fclose(fp);
    }
    else
    {
        printf("读取失败，请检查文件路径以及文件名称是否符合规范\n");
    }

    return err;
}

int file_write(Id pro_res, char f_path[])
{
    FILE* fp;
    char res[v_len];
    fp = fopen(f_path, "a");
    if (pro_write == 0)
    {
        fputs("\n\n", fp);
    }

    if (pro_res.id_type_float == 1)
    {
        sprintf(res, "%.2f", pro_res.id_num.id_float);
        fputs(res, fp);
        fputs("\n", fp);
    }
    else
    {
        sprintf(res, "%d", pro_res.id_num.id_int);
        fputs(res, fp);
        fputs("\n", fp);
    }

    fclose(fp);
    return 1;
}

//栈相关函数实现
void init_stack(Stack* stack)
{
    stack->base = (Token*)malloc(MAX * sizeof(Stack));
    stack->size = 0;
    stack->top = NULL;
}

void free_stack(Stack* stack)
{
    free(stack->base);
}

void push(Stack* stack, Token t)
{
    int size = stack->size;

    if (size >= MAX)
    {
        error("error: stack overflow");
    }

    stack->base[size] = t;
    stack->size++;
    stack->top = stack->base + stack->size - 1;
}

Token* pop(Stack* stack)
{
    if (stack->size == 0)
    {
        error("error: can't pop empty stack");
    }

    Token* top_token = top(stack);
    stack->top = stack->top - 1;
    stack->size--;
    return top_token;
}

Token* top(Stack* stack)
{
    return stack->top;
}

void inverse(Stack* stack)
{
    Stack s2;
    init_stack(&s2);

    for (size_t i = stack->size; i > 0; i--)
    {
        Token* t = pop(stack);
        push(&s2, *t);
    }
    free_stack(stack);
    stack->top = s2.top;
    stack->base = s2.base;
    stack->size = s2.size;
}

// 错误消息
void error(const char* message)
{

    fprintf(stderr, "error(line %d ):\n%s\n", line, message);
    printf("程序出错,无法到相应文件查询到计算结果\n");
    system("PAUSE");
    exit(1);
}

int scan_token(char pro[], int pro_len, int offset, Token* current)//扫描函数，扫描下一token
{
    char tem_char = pro[offset];
    for (int i = 0; i < v_len; i++)
    {
        current->value.v_val[i] = '\0';
    }//初始化
    while (offset < pro_len)
    {
        if (tem_char == '.')
        {
            current->Token_tag = ALLEND;
            return offset + 1;
        }
        if (tem_char >= 'a' && tem_char <= 'z' || tem_char >= 'A' && tem_char <= 'Z')//如果是字符串
        {
            char tem_vari[v_len] = { 0 };
            int tem_vari_len = 0;
            do
            {

                tem_vari[tem_vari_len] = tem_char;
                offset++;
                tem_vari_len++;
                tem_char = pro[offset];
            } while (tem_char >= '0' && tem_char <= '9' || tem_char >= 'a' && tem_char <= 'z' || tem_char >= 'A' && tem_char <= 'Z');

            if (strcmp(tem_vari, "int") == 0 || strcmp(tem_vari, "float") == 0)
            {
                current->Token_tag = TYPE;
                strcpy(current->value.v_val, tem_vari);
                return offset;
            }
            if (strcmp(tem_vari, "write") == 0)
            {
                current->Token_tag = WRITE;
                strcpy(current->value.v_val, tem_vari);
                return offset;
            }

            current->Token_tag = VARIABLE;
            strcpy(current->value.v_val, tem_vari);
            return offset;
        }
        if (tem_char == '+' || tem_char == '*' || tem_char == '/' || tem_char == '-' || tem_char == '(' || tem_char == ')')
        {
            current->Token_tag = SIGN;
            current->value.sign = tem_char;
            return offset + 1;
        }
        if (tem_char == ';')
        {
            current->Token_tag = END;
            return offset + 1;
        }
        if (tem_char == '\n')
        {
            line++;
            offset++;
            tem_char = pro[offset];
            continue;
        }
        if (tem_char == ' ')
        {
            offset++;
            tem_char = pro[offset];
            continue;
        }
        if (tem_char == '\0')
        {
            error("error: no .");
            current->Token_tag = ALLEND;
            return offset + 1;
        }
        if (tem_char == '=')
        {
            current->Token_tag = EQUAL;
            return offset + 1;
        }

        if (tem_char >= '0' && tem_char <= '9')
        {
            int num_type = 0;
            int num_int = 0;
            float num_float = 0;

            char tem_num[v_len] = "\0";
            int i = 0;

            do
            {
                if (i >= v_len - 1)
                {
                    error("error: digital accuracy exceeds storage range ");
                }
                tem_num[i] = tem_char;
                i++;
                offset++;
                tem_char = pro[offset];
                if (tem_char == '.')
                    if (num_type == 0)//只能接受一个小数点
                        num_type = i;
                    else
                        break;
            } while (tem_char >= '0' && tem_char <= '9' || tem_char == '.');

            if (num_type == 0 || tem_num[num_type + 1] == '\0')
            {
                if (tem_num[0] == '0'&&tem_num[1]!='\0')
                {
                    error("error: decimal int type cannot start with 0 \n");
                }
                if (num_type == 0)
                {
                    current->Token_tag = INT;
                    current->value.i_val = atoi(tem_num);
                    return offset;
                }
                else
                {
                    tem_num[num_type] = '\0';
                    current->Token_tag = INT;
                    current->value.i_val = atoi(tem_num);
                    return offset - 1;
                }

            }
            else
            {
                if (tem_num[0] == '0' && tem_num[1] != '.')
                {
                    error("error: decimal float type cannot start with 0");
                }
                current->Token_tag = FLOAT;
                current->value.f_val = (float)(atof(tem_num));
                return offset;
            }

        }

        error("error: syntax error");
    }
    if (tem_char == '/0')
    {
        current->Token_tag = END;
        return offset;
    }//彻底结束
    return -1;
}

int get_op_level(int op) {
    switch (op)
    {
    case '(':
        return 1;
    case '+':
    case '-':
        return 2;
    case '*':
    case '/':
        return 3;
    case ')':
        return 4;
    default:
        error("error: bad op");
        return 0;
    }
}




void cal_token(Token* t1, Token* t2, int sign, Token* result)
{
    if (t1->Token_tag == FLOAT || t2->Token_tag == FLOAT)
    {//浮点数的计算
        result->Token_tag = FLOAT;
        float v1, v2;
        v1 = t1->Token_tag == FLOAT ? t1->value.f_val : t1->value.i_val;
        v2 = t2->Token_tag == FLOAT ? t2->value.f_val : t2->value.i_val;

        switch (sign)
        {
        case '+':
            result->value.f_val = v1 + v2;
            return;
        case '-':
            result->value.f_val = v1 - v2;
            return;
        case '*':
            result->value.f_val = v1 * v2;
            return;
        case '/':
            if (v2 == 0.0f)
            {
                error("error: divide by zero");
            }
            result->value.f_val = v1 / v2;
            return;
        default:
            return;
        }
    }
    else {//整数的计算
        result->Token_tag = INT;
        int v1, v2;
        v1 = t1->value.i_val;
        v2 = t2->value.i_val;
        switch (sign)
        {
        case '+':
            result->value.i_val = v1 + v2;
            return;
        case '-':
            result->value.i_val = v1 - v2;
            return;
        case '*':
            result->value.i_val = v1 * v2;
            return;
        case '/':
            if (v2 == 0)
            {
                error("error: divide by zero");
            }
            result->value.i_val = v1 / v2;
            return;
        default:
            return;
        }
    }
}

int cal(char pro[], int offset, int pro_len, Token* cal_result)
{
    int tem_var;
    Token current;
    int cal_allend = 0;
    int LR_fac = 0;
    for (int i = 0; i < v_len; i++)
    {
        current.value.v_val[i] = '\0';
    }
    current.Token_tag = START;

    Stack cal_post;
    Stack cal_ops;
    init_stack(&cal_post);
    init_stack(&cal_ops);

    do
    {
        offset = scan_token(pro, pro_len, offset, &current);
        switch (current.Token_tag)
        {
        case START:
        {
            error("error:find no token");
            break;
        }
        case END:
        {
            break;
        }
        case ALLEND:
        {
            cal_allend = -1;
            break;
        }
        case INT:
        case FLOAT:
        {
            push(&cal_post, current);
            break;
        }
        case SIGN:
        {
            switch (current.value.sign)
            {
            case LP:
            {
                push(&cal_ops, current);
                LR_fac++;
                break;
            }
            case RP:
            {
                LR_fac--;
                do
                {
                    Token* op;
                    if (cal_ops.size == 0)//无匹配左括号
                    {
                        error("error: parentheses mismatch");
                    }
                    op = pop(&cal_ops);
                    push(&cal_post, *op);
                } while (top(&cal_ops)->value.sign != LP);
                pop(&cal_ops);
                break;
            }
            case PLUS:
            case MINUS:
            case MUL:
            case DIV:
            {
                if (cal_ops.size == 0)
                {
                    push(&cal_ops, current);
                    break;
                }
                int top_op = top(&cal_ops)->value.sign;
                if (get_op_level(current.value.sign) > get_op_level(top_op))
                {
                    push(&cal_ops, current);
                    break;

                }
                else
                {
                    while (cal_ops.size != 0)
                    {
                        Token* op;
                        top_op = top(&cal_ops)->value.sign;
                        if (get_op_level(current.value.sign) <= get_op_level(top_op))
                        {
                            op = pop(&cal_ops);
                            push(&cal_post, *op);
                        }
                        else
                        {
                            break;
                        }
                    }
                    push(&cal_ops, current);
                    break;
                }
            }
            default:
                break;
            }
            break;
        }
        case VARIABLE:
        {
            int j = 0;
            int tem_id_num = -1;
            for (int i = 0; i < id_sum; i++)
            {
                if (strcmp(current.value.v_val, pro_id[i].id_name) == 0)
                {
                    tem_id_num = i;
                    j = 1;
                    break;
                }               
            }
            if (j == 0)
            {
                error("error: undefined identifier");
            }
            if (pro_id[tem_id_num].id_num_bool == 0)
            {
                error("error: variable is not assigned \n");
            }          
            Token tem_op;
            if (pro_id[tem_id_num].id_type_float == 1)
            {
                tem_op.Token_tag = FLOAT;
                tem_op.value.f_val = pro_id[tem_id_num].id_num.id_float;
            }
            else
            {
                tem_op.Token_tag = INT;
                tem_op.value.i_val = pro_id[tem_id_num].id_num.id_int;
            }
            push(&cal_post, tem_op);
            break;
        }
        default:
        {
            error("error : syntax error");
            break;
        }
        }
    } while (current.Token_tag != END && current.Token_tag != ALLEND);
    if (LR_fac != 0)
    {
        error("error:  parentheses mismatch");
    }
    while (cal_ops.size != 0)
    {
        Token* op = pop(&cal_ops);
        push(&cal_post, *op);
    }


    free_stack(&cal_ops);
    inverse(&cal_post);

    //for (int i = 0, j = cal_post.size; i < j; i++)
    //{
    //    Token* op = pop(&cal_post);
    //    printf("1");
    //}
    //exit(1);

    Stack tmp;
    init_stack(&tmp);

    while (cal_post.size != 0)
    {
        int tag = top(&cal_post)->Token_tag;
        switch (tag)
        {
        case INT:
        case FLOAT:
        {
            push(&tmp, *pop(&cal_post));
            break;
        }
        case SIGN:
        {
            Token t;
            int sign = pop(&cal_post)->value.sign;
            switch (sign)
            {
            case'+':
            case '-':
            case '*':
            case '/':
            {
                Token* value2 = pop(&tmp);
                Token* value1 = pop(&tmp);
                cal_token(value1, value2, sign, &t);
                push(&tmp, t);
                break;
            }
            }
            break;
        }
        default:
        {
            error("error: syntax error");
            break;
        }
        }
    }
    if (tmp.size != 1)
    {
        printf("%d\n", tmp.size);
        error("error: bad expr");
    }
    Token* result = pop(&tmp);
    cal_result->Token_tag = result->Token_tag;
    cal_result->value = result->value;
    if (cal_allend == -1)
        return cal_allend;
    else
        return offset;

    if (cal_allend == -1)
        return cal_allend;
    else
        return offset;
}