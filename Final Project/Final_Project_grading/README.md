# Final Project Grading Script
We will use this script to judge your program.  
__Please make sure your program can be executed by this script.__

#
## Preparing
* Step1:  
    Put all testcases in the directory `testcases`.

* Step2:  
    Put your compressed file in the directory `group/group1`.  
    The whole path is as follow: 

    ```
    Final_Project_grading/group/group1/CS6135_Final_Project.tar.gz
    ```

### Notice:  
__Do not put your original directory here__ because it will remove all directories before unzipping the compressed file.

#
## Working Flow
* Step1:  
    Go into directory `Final_Project_grading` and change the permission of `Final_Project_grading.sh`.

    ```shell
    $ chmod 500 Final_Project_grading.sh
    ```

* Step2:  
    Run `Final_Project_grading.sh`.

    ```shell
    $ bash Final_Project_grading.sh
    ```

* Step3:  
    Check your output.
    * If the status is __success__, it means your program finished in time and your output is correct. e.g.:

        ```
        grading on group1:
          testcase |           score |    runtime | status
             case1 |               0 |       0.15 | success
             case2 |               0 |       0.05 | success
             case3 |               0 |       0.07 | success
             case4 |               0 |       0.40 | success
             case5 |               0 |       0.77 | success
             case6 |               0 |       1.34 | success
            case3B |               0 |       0.07 | success
            case4B |               0 |       0.39 | success
            case5B |               0 |       0.78 | success
            case6B |               0 |       1.37 | success
        ```
    * If the status is not __success__, it means your program fails in this case. e.g.:

        ```
        grading on group1:
          testcase |      score |    runtime | status
             case1 |       fail |       0.05 | case1 has something wrong.
             case2 |       fail |       0.05 | case2 has something wrong.
             case3 |       fail |       0.06 | case3 has something wrong.
             case4 |       fail |       0.24 | case4 has something wrong.
             case5 |       fail |       0.44 | case5 has something wrong.
             case6 |       fail |       0.80 | case6 has something wrong.
            case3B |       fail |       0.06 | case3B has something wrong.
            case4B |       fail |       0.25 | case4B has something wrong.
            case5B |       fail |       0.43 | case5B has something wrong.
            case6B |       fail |       0.80 | case6B has something wrong.
        ```
