#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#define MAX_LINE_LENGTH 150
#define BUFFER_SIZE (3 * (MAX_LINE_LENGTH))


char *itoa(int num, char *str, int base) {
    int i = 0;
    int is_negative = 0;
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
    if (num < 0 && base == 10) {
        is_negative = 1;
        num = -num;
    }
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }
    if (is_negative) {
        str[i++] = '-';
    }
    str[i] = '\0';
    int j;
    for (j = 0; j < i / 2; j++) {
        char temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }
    return str;
}
void alarm_hand (int s) {}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        perror("not enough arguments");
        exit(-1);
        return 1;
    }

    int wrote = 0;
    char* grade_str[] = {"NO_C_FILE", "COMPILATION_ERROR", "TIMEOUT","WRONG","SIMILAR", "EXCELLENT"};


    int grade_int[] = {0, 10, 20, 50, 75, 100};
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        //error
        perror("Error in: open");
        exit(-1);
    }
    pid_t pid3 = fork();
    if(pid3 == 0) {
        char *argsgcc[] = {"gcc", "ex21.c", "-o", "comp.out", NULL};
        execvp(argsgcc[0], argsgcc);
        perror("Error in: execvp");
        exit(-1);

    } else if(pid3 > 0) {
        int k;
        wait(&k);
        char cwd[1024];
        char comp_out_path[1024];
        char path_to_correct_output[1024];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("Error in: getcwd");
            exit(-1);
        }
        strcpy(comp_out_path, cwd);
        strcpy(path_to_correct_output, cwd);

        strcat(comp_out_path, "/comp.out");
        char buffer[BUFFER_SIZE];
        int bytes_read = read(fd, buffer, BUFFER_SIZE-1);
        if (bytes_read < 0) {
            //error
            close(fd);
            perror("Error in: read");
            exit(-1);
        }
        buffer[bytes_read] = '\0';
        close(fd);
        char lines[3][MAX_LINE_LENGTH];
        char *line = strtok(buffer, "\r\n");
        int line_count = 0;
        while (line != NULL && line_count < 3) {
            size_t line_len = strlen(line);
            if (line_len > 0 && line[line_len-1] == '\r') {
                line[line_len-1] = '\0';  // replace trailing CR with null character
            }
            strcpy(lines[line_count], line);
            line = strtok(NULL, "\r\n");
            line_count++;
        }
        close(fd);
        int fd_grades = open("results.csv", O_CREAT | O_WRONLY | O_APPEND, 0644);
        if (fd_grades == -1) {
            perror("Error in: open");
            exit(-1);
        }
        int fd_error = open("errors.txt", O_CREAT | O_WRONLY | O_APPEND, 0644);
        if (fd_error == -1) {
            perror("Error in: open");
            exit(-1);
        }
//        if(close(fd_error) == -1) {
//            perror("Error in: close");
//            exit(-1);
//        }

        DIR *dir = opendir(lines[0]);
        if (dir == NULL) {
            perror("Not a valid directory");
            exit(-1);
        }
        if (chdir(lines[0]) == -1) { // change directory to the first line of the file
            perror("Not a valid directory");
            exit(-1);
        }


        char path_to_input[1024];
        strcpy(path_to_input, path_to_correct_output);
        strcat(path_to_input, "/");
        strcat(path_to_input, lines[1]);
        int fd_input = open(path_to_input, O_RDONLY);
        if (fd_input == -1) {
            perror("Input file not exist");
            exit(-1);
        } else {
            close(fd_input);
        }




        strcat(path_to_correct_output, "/");
        strcat(path_to_correct_output, lines[2]);


        int fd_output = open(path_to_correct_output, O_RDONLY);
        if (fd_output == -1) {
            perror("Output file not exist");
            exit(-1);
        } else {
            close(fd_output);
        }

        int grade = 0;
        struct dirent *entry;
        char path_to_students[1024];
        strcpy(path_to_students, cwd);
        strcat(path_to_students, "/");
        strcat(path_to_students, lines[0]);
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char path_to_student[1024];
                strcpy(path_to_student, path_to_students);
                strcat(path_to_student, "/");
                strcat(path_to_student, entry->d_name);
                // if entry is a directory and not the current or parent directory
                if (chdir(entry->d_name) == -1) {
                    perror("Error in: chdir");
                    exit(-1);
                }
                DIR *subdir = opendir(".");
                if (subdir == NULL) {
                    perror("Error in: opendir");
                    exit(-1);
                }
                int c_file_exist = 0;
                struct dirent *subentry;
                while ((subentry = readdir(subdir)) != NULL) {
                    if (strcmp(subentry->d_name + strlen(subentry->d_name) - 2, ".c") == 0 && subentry->d_type != DT_DIR) { //if its c file
                        c_file_exist = 1;
//                        printf("the name of the file: %s\n", subentry->d_name);
                        strcat(path_to_student, "/");
                        strcat(path_to_student, subentry->d_name);

                        char *args[] = {"gcc", path_to_student, NULL};
                        pid_t pid = fork();
                        if (pid == 0) {
                            dup2(fd_error, 2);
                            close(fd_error);
                            execvp("gcc", args);
                            perror("Error in: execvp");
                            exit(-1);
                        } else if (pid > 0) {

                            int status;
                            if (wait(&status) == -1) {
                                perror("Error in: wait");
                                exit(-1);
                            }

                            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                                // Compilation succeeded
                                //fork and run



                                /* running the program */
                                pid_t pid1 = fork();
                                if (pid1 == 0) {
                                    //opening the input file
//                                    char cwd_output1[1024];
//                                    getcwd(cwd_output1, strlen(cwd_output1));
//                                    printf("before error 4 %s\n", cwd_output1);
                                    int fd_input = open(path_to_input, O_RDONLY);
                                    if (fd_input == -1) {
                                        perror("Error in: open");
                                        exit(-1);
                                    }
                                    int fd_output = open("output.txt", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
                                    if (fd_output == -1) {
                                        perror("Error in: open");


                                        exit(-1);
                                    }
                                    //check if opened correctly

                                    if (dup2(fd_input, STDIN_FILENO) == -1 || dup2(fd_output, STDOUT_FILENO) == -1) {
                                        perror("Error in: dup2");
                                        exit(-1);
                                    }
                                    char *args[] = {"./a.out", NULL};
                                    signal(SIGALRM, alarm_hand);
                                    alarm(5);
                                    execvp(args[0], args);
                                    perror("Error in: execvp");
                                    exit(-1);
                                } else if (pid1 > 0) {
                                    int s;
                                    if (wait(&s) == -1) {
                                        perror("Error in: wait");
                                        exit(-1);
                                    }
                                    alarm(0);
                                    if (WIFSIGNALED(s)) {
//                                        close(fd_input);
//                                        close(fd_output);
                                        grade = 2;
                                        char str_grade[10];
                                        itoa(grade_int[grade], str_grade, 10);
                                        char res[256] = "";
                                        strcat(res, entry->d_name);
                                        strcat(res, ",");
                                        strcat(res, str_grade);
                                        strcat(res, ",");
                                        strcat(res, grade_str[grade]);
                                        strcat(res, "\n");
                                        write(fd_grades, res, strlen(res));
                                        char file_name[1024];
                                        if (getcwd(file_name, sizeof(file_name)) != NULL) {

                                        } else {
                                            perror("Error in: getcwd");
                                            return -1;
                                        }

                                        char outname[1024];
                                        strcpy(outname, file_name);
                                        strcat(outname, "/a.out");
                                        strcat(file_name, "/output.txt");
                                        int result = remove(file_name);
                                        if (result == 0) {
                                        } else {
                                            perror("Error in: remove");
                                            return -1;
                                        }

                                        int result1 = remove(outname);
                                        if (result1 == 0) {
                                        } else {
                                            perror("Error in: remove");
                                            return -1;
                                        }
                                        break;
                                    }
                                    //fork and compare
                                    pid_t pid2 = fork();
                                    if (pid2 == 0) {
                                        char path_to_output_txt[1024];
                                        if (getcwd(path_to_output_txt, sizeof(path_to_output_txt)) == NULL) {
                                            perror("Error in: getcwd");
                                            exit(-1);
                                        }
                                        strcat(path_to_output_txt, "/output.txt");

                                        char *args[] = {comp_out_path, path_to_output_txt, path_to_correct_output,
                                                        NULL};
                                        execvp(args[0], args);
                                        perror("Error in: execvp");
                                        exit(-1);
                                    } else if (pid2 > 0) {
                                        int st;
                                        if (wait(&st) == -1) {
                                            perror("Error in: wait");
                                            exit(-1);
                                        }
                                        int exit_status = WEXITSTATUS(st);
                                        if (exit_status == 3) {
                                            grade = 4;
                                        } else if (exit_status == 2) {
                                            grade = 3;
                                        } else {
                                            grade = 5;
                                        }
                                        // printf("grade %d %s\n", grade,entry->d_name );
                                        char str_grade[10];
                                        itoa(grade_int[grade], str_grade, 10);
                                        char res[256] = "";
                                        strcat(res, entry->d_name);
                                        strcat(res, ",");
                                        strcat(res, str_grade);
                                        strcat(res, ",");
                                        strcat(res, grade_str[grade]);
                                        strcat(res, "\n");
                                        write(fd_grades, res, strlen(res));




                                        char file_name[1024];
                                        if (getcwd(file_name, sizeof(file_name)) != NULL) {

                                        } else {
                                            perror("Error in: getcwd");
                                            return -1;
                                        }
                                        char outname[1024];
                                        strcpy(outname, file_name);
                                        strcat(outname, "/a.out");
                                        strcat(file_name, "/output.txt");
                                        int result = remove(file_name);
                                        if (result == 0) {
                                        } else {
                                            perror("Error in: remove");
                                            return -1;
                                        }

                                        int result1 = remove(outname);
                                        if (result1 == 0) {
                                        } else {
                                            perror("Error in: remove");
                                            return -1;
                                        }

                                    } else {
                                        perror("Error in: fork");
                                        exit(-1);
                                    }


                                } else {
                                    perror("Error in: fork");
                                    exit(-1);
                                }


                            } else {
                                // Compilation failed
                                //printf("comp error\n");
                                grade = 1;
//                                printf("grade %d %s\n", grade,entry->d_name );
                                //write grade
                                char str_grade[10];
                                itoa(grade_int[grade], str_grade, 10);
                                char res[256] = "";
                                strcat(res, entry->d_name);
                                strcat(res, ",");
                                strcat(res, str_grade);
                                strcat(res, ",");
                                strcat(res, grade_str[grade]);
//                                strcat(res, entry->d_name);
                                strcat(res, "\n");
                                write(fd_grades, res, strlen(res));
//                                char file_name[1024];
//                                if (getcwd(file_name, sizeof(file_name)) != NULL) {
//
//                                } else {
//                                    perror("Error in: getcwd");
//                                    return -1;
//                                }
//                                printf("the cwd is %s\n", file_name);
//                                char outname[1024];
//                                strcpy(outname, file_name);
//                                strcat(outname, "/a.out");
//                                strcat(file_name, "/output.txt");
//                                int result = remove(file_name);
//                                if (result == 0) {
//                                } else {
//                                    perror("Error3 in: remove");
//                                    return -1;
//                                }
//
//                                int result1 = remove(outname);
//                                if (result1 == 0) {
//                                } else {
//                                    perror("Error3 in: remove");
//                                    return -1;
//                                }
                            }
                        } else {
                            perror("Error in: fork");
                            exit(-1);
                        }
                    }

                }
                if(!c_file_exist) {
                    grade = 0;
                    //printf("grade %d %s\n", grade,entry->d_name );
                    //write the grade
                    char str_grade[10];
                    itoa(grade_int[grade], str_grade, 10);
                    char res[256] = "";
                    strcat(res, entry->d_name);
                    strcat(res, ",");
                    strcat(res, str_grade);
                    strcat(res, ",");
                    strcat(res, grade_str[grade]);
                    strcat(res, "\n");
                    write(fd_grades, res, strlen(res));

                }
                if(closedir(subdir) == -1) {
                    perror("Error in: closedir");
                    exit(-1);
                }

                if(chdir("..")) {
                    perror("Error in: chdir");
                    exit(-1);
                }

            }
        }
        if(closedir(dir) == -1) {
            perror("Error in: closedir");
            exit(-1);
        }
        if(close(fd_grades) == -1) {
            perror("Error in: close");
            exit(-1);
        }
        if(close(fd_error) == -1) {
            perror("Error in: close");
            exit(-1);
        }

    } else {
        perror("Error in: fork");
        exit(-1);
    }

    return 0;
}
