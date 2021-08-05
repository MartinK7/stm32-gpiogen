#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

#define assert(_expr_,_str_) if(!(_expr_)){fprintf(stderr,"Error: "_str_"\n");exit(1);}
#define assertArgs(_expr_,_str_,...) if(!(_expr_)){fprintf(stderr,"Error: "_str_"\n",__VA_ARGS__);exit(1);}
#define cmp(_str_) (strcasecmp(buf,_str_)==0)

typedef struct {
    char name[17+1];
    char port;
    int pin;
    int mode;
    int otyper;
    int ospeedr;
    int pupdr;
    int afr;
    int ready;
} config_t;

typedef enum {
    STYLE_NOTSET,
    STYLE_PONY
} style_e;

typedef enum {
    ORIENTATION_NOTSET,
    ORIENTATION_VER,
    ORIENTATION_HOR
} orientation_e;

typedef enum {
    NEXT,
    LINE,
    END
} state_e;

int getNum(char *str) {
    int ret;
    if(str[0] == '0' && str[1] == 'x') {
        sscanf(&str[2], "%x", &ret);
        return ret;
    } else {
        sscanf(str, "%d", &ret);
        return ret;
    }
}

int strspeccmp(char *str1, char *str2) {
    int len = strlen(str1) < str2 ? strlen(str1) : strlen(str1);
    int ok = 0;
    for(int i=0; i<len; ++i) {
        if(tolower(str1[i]) == tolower(str2[i]))
            ++ok;
        else
            break;
    }
    return ok;
}

state_e parse(FILE *f, char *out, int bufOutLen) {
    if(feof(f)) {
        out[0] = 0;
        return END;
    }

    for(int i=0; i<bufOutLen; ++i) {
        int c = fgetc(f);
        if(c == EOF) {
            out[i] = 0;
            return END;
        } else if(c == '\n') {
            out[i] = 0;
            return LINE;
        } else if(c == ',') {
            out[i] = 0;
            return NEXT;
        } else if(c == ' ' || c == '\t') {
            // Ignore
            --i;
        } else {
            out[i] = c;
        }
    }
    assert(0,"Line too long!");
}

state_e skip(FILE *f) {
    int c;
    do {
        c = fgetc(f);
    } while(c != EOF && c != '\n');
    if(c == '\n') {
        return LINE;
    }
    return END;
}

int main(int argc, char *argv[]) {
    // Get file names
    assert(argc==3,"In/Out file not specified!");
    const char *f_in_path = argv[1];
    const char *f_out_path = argv[2];

    // Open files
    FILE *f_in = fopen(f_in_path,"rb");
    FILE *f_out = fopen(f_out_path,"wb");

    // Test successful
    assertArgs(f_in,"Open file '%s'",f_in_path);
    assertArgs(f_out,"Open file '%s'",f_out_path);

    int listCount = 0;
    config_t list[64] = {0};
    char buf[512] = {0};
    state_e st;
    style_e style = STYLE_NOTSET;
    int inlineCount = 0;

    while((st = parse(f_in, buf, 512)) != END) {
        do {
            // Skip comment
            if(buf[0] == '#') {
                st = skip(f_in);
                break;
            }

            // Compare special commands
            if cmp("style") {
                assertArgs(inlineCount==0,"Bad location '%s'", buf);
                parse(f_in, buf, 512);
                if cmp("pony") {
                    // Other styles is not supported for now
                    style = STYLE_PONY;
                    st = skip(f_in);
                } else {
                    assertArgs(0, "Unknown style '%s'", buf);
                }
                break;
            } else if cmp("orientation") {
                assertArgs(inlineCount==0,"Bad location '%s'", buf);
                parse(f_in, buf, 512);
                if cmp("horizontal") {
                    // Other orientation is not supported for now
                    style = ORIENTATION_HOR;
                    st = skip(f_in);
                } else {
                    assertArgs(0, "Unknown orientation '%s'", buf);
                }
                break;
            }

            int p;
            // Standard GPIO settings
            switch (inlineCount) {
                case 0: // Name
                    // Skip empty line
                    if(strlen(buf) == 0) {
                        st = skip(f_in);
                        break;
                    }
                    printf("Name: %s\n", buf);
                    assert(list[listCount].ready != -1, "Configuration at some line is incomplete!")
                    list[listCount].ready = -1;
                    strncpy(list[listCount].name, buf, 14);
                    break;
                case 1: // Port
                    p = toupper(buf[strlen(buf)-1]);
                    printf("     Port: %c\n", p);
                    assertArgs(p >= 'A' && p <= 'Z', "Bad port '%s' specifed!", buf);
                    list[listCount].port = p;
                    break;
                case 2: // Pin
                    list[listCount].pin = getNum(buf);
                    printf("     Pin: %d\n", list[listCount].pin);
                    assertArgs(list[listCount].pin >= 0 && list[listCount].pin <= 15,
                               "Bad pin '%s' number specified!", buf)
                    break;
                case 3: // Mode
                    if(strspeccmp(buf,"input")>=2) {
                        printf("     Mode: input\n");
                        list[listCount].mode = 0;
                    } else if(strspeccmp(buf,"output")>=1) {
                        printf("     Mode: output\n");
                        list[listCount].mode = 1;
                    } else if(strspeccmp(buf,"alternate")>=2) {
                        printf("     Mode: alternate\n");
                        list[listCount].mode = 2;
                    } else if(strspeccmp(buf,"analog")>=2) {
                        printf("     Mode: analog\n");
                        list[listCount].mode = 3;
                    } else {
                        assertArgs(0, "Unknown mode '%s'", buf);
                    }
                    break;
                case 4: // Otyper
                    if(strspeccmp(buf,"push_pull")>=1) {
                        printf("     Otyper: push_pull\n");
                        list[listCount].otyper = 0;
                    } else if(strspeccmp(buf,"open_drain")>=1) {
                        printf("     Otyper: open_drain\n");
                        list[listCount].otyper = 1;
                    } else {
                        assertArgs(0, "Unknown otyper '%s'", buf);
                    }
                    break;
                case 5: // Ospeedr
                    if(strspeccmp(buf,"low")>=2) {
                        printf("     Ospeedr: low\n");
                        list[listCount].ospeedr = 0;
                    } else if(strspeccmp(buf,"medium")>=1) {
                        printf("     Ospeedr: medium\n");
                        list[listCount].ospeedr = 1;
                    } else if(strspeccmp(buf,"high")>=1) {
                        printf("     Ospeedr: high\n");
                        list[listCount].ospeedr = 2;
                    }  else if(strspeccmp(buf,"veryhigh")>=1) {
                        printf("     Ospeedr: veryhigh\n");
                        list[listCount].ospeedr = 3;
                    } else {
                        assertArgs(0, "Unknown ospeedr '%s'", buf);
                    }
                    break;
                case 6: // Pupdr
                    if(strspeccmp(buf,"no")>=1) {
                        printf("     Pupdr: no\n");
                        list[listCount].pupdr = 0;
                    } else if(strcasecmp(buf,"pull_up")==0 || strcasecmp(buf,"pp")==0) {
                        printf("     Pupdr: pull_up\n");
                        list[listCount].pupdr = 1;
                    } else if(strcasecmp(buf,"pull_down")==0 || strcasecmp(buf,"pu")==0) {
                        printf("     Pupdr: pull_down\n");
                        list[listCount].pupdr = 2;
                    } else {
                        assertArgs(0, "Unknown pupdr '%s'", buf);
                    }
                    break;
                case 7: // Afr
                    list[listCount].afr = getNum(buf);
                    printf("     Afr: %d\n", list[listCount].afr);
                    assertArgs(list[listCount].afr >= 0 && list[listCount].afr <= 15,
                               "Bad afr '%s' number specified!", buf)

                    list[listCount].ready = 1;
                    listCount++;
                    st = skip(f_in);
                    break;
            }
        } while (0);

        switch (st) {
            case NEXT:
                inlineCount++;
                break;
            case END:
            case LINE:
                inlineCount = 0;
                break;
            default:
                assert(0, "Internal error!");
        }
    }

    const char *init[13] = {
            "//                  ",
            "->AFR[0]  &= ~(",
            "->AFR[0]  |=  (",
            "->AFR[1]  &= ~(",
            "->AFR[1]  |=  (",
            "->MODER   &= ~(",
            "->MODER   |=  (",
            "->OSPEEDR &= ~(",
            "->OSPEEDR |=  (",
            "->OTYPER  &= ~(",
            "->OTYPER  |=  (",
            "->PUPDR   &= ~(",
            "->PUPDR   |=  ("
    };

    char print[13][1024];
    char *print_o[13];
    char portsel = '0';
    int cnt = 0;
    do {
        if(portsel != list[cnt].port) {
            // Flush
            if(cnt > 0) {
                fprintf(f_out, "%s\n", print[0]);
                for(int i=1; i<13; ++i) {
                    fprintf(f_out, "%s0);\n", print[i]);
                }
                fprintf(f_out, "\n");
            }
            // Init
            portsel = list[cnt].port;
            for(int i=0; i<13; ++i)
                print_o[i] = &print[i][0];
            for(int i=1; i<13; ++i) {
                sprintf(print_o[i], "GPIO%c", portsel);
                print_o[i] += 5;
            }
            for(int i=0; i<13; ++i) {
                strcpy(print_o[i], init[i]);
                print_o[i] += strlen(init[i]);
            }
        }
        // Add
        sprintf(print_o[0], "%s%*s", list[cnt].name, 15-strlen(list[cnt].name), "");print_o[0]+=15;
        if(list[cnt].pin < 8) {
            sprintf(print_o[1], "0xFU<<(%*d-0)*4|", 2, list[cnt].pin);
            print_o[1]+=15;
            sprintf(print_o[2], "0x%XU<<(%*d-0)*4|", list[cnt].afr, 2, list[cnt].pin);
            print_o[2]+=15;
            sprintf(print_o[3], "0             |");
            print_o[3]+=15;
            sprintf(print_o[4], "0             |");
            print_o[4]+=15;
        } else {
            sprintf(print_o[1], "0             |");
            print_o[1]+=15;
            sprintf(print_o[2], "0             |");
            print_o[2]+=15;
            sprintf(print_o[3], "0xFU<<(%*d-8)*4|", 2, list[cnt].pin);
            print_o[3]+=15;
            sprintf(print_o[4], "0x%XU<<(%*d-8)*4|", list[cnt].afr, 2, list[cnt].pin);
            print_o[4]+=15;
        }
        sprintf(print_o[5], "0x3U<<(%*d*2)  |", 2, list[cnt].pin);print_o[5]+=15;
        sprintf(print_o[6], "0x%XU<<(%*d*2)  |", list[cnt].mode, 2, list[cnt].pin);print_o[6]+=15;
        sprintf(print_o[7], "0x3U<<(%*d*2)  |", 2, list[cnt].pin);print_o[7]+=15;
        sprintf(print_o[8], "0x%XU<<(%*d*2)  |", list[cnt].ospeedr, 2, list[cnt].pin);print_o[8]+=15;
        sprintf(print_o[9], "0x1U<<(%*d*1)  |", 2, list[cnt].pin);print_o[9]+=15;
        sprintf(print_o[10], "0x%XU<<(%*d*1)  |", list[cnt].otyper, 2, list[cnt].pin);print_o[10]+=15;
        sprintf(print_o[11], "0x3U<<(%*d*2)  |", 2, list[cnt].pin);print_o[11]+=15;
        sprintf(print_o[12], "0x%XU<<(%*d*2)  |", list[cnt].pupdr, 2, list[cnt].pin);print_o[12]+=15;

    } while ((++cnt)<listCount);

    // Flush
    if(cnt > 0) {
        fprintf(f_out, "%s\n", print[0]);
        for(int i=1; i<13; ++i) {
            fprintf(f_out, "%s0);\n", print[i]);
        }
    }

    // Close files
    fclose(f_in);
    fclose(f_out);

    return 0;
}