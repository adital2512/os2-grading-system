#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h> // add header file for errno

int are_files_identical(const char* path1, const char* path2) {
    int fd1 = open(path1, O_RDONLY);
    int fd2 = open(path2, O_RDONLY);
    if (fd1 == -1 || fd2 == -1) {
        perror("Error in: open");
        return -1;
    }
    int result = 1;
    char byte1, byte2;
    while (1) {
        ssize_t count1 = read(fd1, &byte1, 1);
        ssize_t count2 = read(fd2, &byte2, 1);
        if (count1 == -1 || count2 == -1) {
            perror("Error in: read");
        return -1;
        }
        if (byte1 != byte2) {
            result = 0;
            break;
        }
        if (count1 == 0 || count2 == 0) {
            break;
        }
    }
    close(fd1);
    close(fd2);
    return result;
}

int are_files_similar(const char* path1, const char* path2) {
    int fd1 = open(path1, O_RDONLY);
    int fd2 = open(path2, O_RDONLY);
    if (fd1 == -1 || fd2 == -1) {
        perror("Error in: open");

        return -1;
    }
    int result = 1;
    char byte1, byte2;
    while (1) {
        ssize_t count1 = read(fd1, &byte1, 1);
        while (count1 == 1 && (byte1 == '\n' || byte1 == ' '|| byte1 == '\r')) {
            count1 = read(fd1, &byte1, 1);
        }
        if (byte1 >= 'A' && byte1 <= 'Z') {
            byte1 += 32;
        }
        ssize_t count2 = read(fd2, &byte2, 1);
        while (count2 == 1 && (byte2 == '\n' || byte2 == ' ' || byte2 == '\r')) {
            count2 = read(fd2, &byte2, 1);
        }
        if (byte2 >= 'A' && byte2 <= 'Z') {
            byte2 += 32;
        }
        if (count1 == -1 || count2 == -1) {
            perror("Error in: open");
            return -1;
        }

//        if (count1 == 0 || count2 == 0) {
//            break;
//        }
        if (count1 == 0 && count2 == 0) {
            result = 1;
            break;
        } else if(count1 == 0) {
            while(count2 == 1 && (byte2 == '\n' || byte2 == ' ' || byte2 == '\r')){
                count2 = read(fd2, &byte2, 1);
            }
            if (count2 == 0) {
                result = 1;
                break;
            }
        } else if(count2 == 0) {
            while(count1 == 1 && (byte1 == '\n' || byte1 == ' ' || byte1 == '\r')){
                count1 = read(fd1, &byte1, 1);
            }
            if (count1 == 0) {
                result = 1;
                break;
            }
        }
        if (byte1 != byte2) {

            result = 0;
            break;
        }
    }

    close(fd1);
    close(fd2);
    return result;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        return -1;
    }
    int identical = are_files_identical(argv[1], argv[2]);
    if (identical == 1) {
//        printf("1\n");
        return 1;
    } else if (identical == -1) {
        return -1;
    } else {
        int similar = are_files_similar(argv[1], argv[2]);
        if (similar == 1) {
            return 3;
        } else if (similar == -1) {
            return -1;
        } else {
//            printf("2\n");
            return 2;
        }
    }
    return 0;
}
