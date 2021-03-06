# Homework 3 Grading Script
We will use this script to judge your program.  
__Please make sure your program can be executed by this script.__

#
## Preparing
* Step1:  
    Go into directory `student` and generate a new directory with your student id.

    ```shell
    $ cd student/
    $ mkdir ${your_student_id}
    $ cd ${your_student_id}/
    ```

    e.g.:

    ```shell
    $ cd student/
    $ mkdir 110062501
    $ cd 110062501/
    ```

* Step2:  
    Put your compressed file in the directory which you just generate.  
    The whole path is as follow: 

    ```
    HW3_grading/student/${your_student_id}/CS6135_HW3_${your_student_id}.tar.gz
    ```

    e.g.:

    ```
    HW3_grading/student/110062501/CS6135_HW3_110062501.tar.gz
    ```

### Notice:  
__Do not put your original directory here__ because it will remove all directory before unzipping the compressed file.

#
## Working Flow
* Step1:  
    Go into directory `HW3_grading` and change the permission of `HW3_grading.sh`.

    ```shell
    $ chmod 500 HW3_grading.sh
    ```

* Step2:  
    Run `HW3_grading.sh`.

    ```shell
    $ bash HW3_grading.sh
    ```

* Step3:  
    Check your output.
    * If status is __success__, it means your program is finish in time and your output is correct. e.g.:

        ```
        grading on 110062501:
        testcase |      ratio | wirelength |    runtime | status
            n100 |       0.15 |     207309 |       4.26 | success
            n200 |       0.15 |     367785 |      25.31 | success
            n300 |       0.15 |     504903 |      78.49 | success
            n100 |        0.1 |     227050 |      11.29 | success
            n200 |        0.1 |     535176 |     128.65 | success
            n300 |        0.1 |     518116 |      82.25 | success
        ```
    * If status is not __success__, it means your program fails in this case. e.g.:

        ```
        grading on 110062501:
        testcase |      ratio | wirelength |    runtime | status
            n100 |       0.15 |       fail |        TLE | n100 failed.
            n200 |       0.15 |       fail |        TLE | n200 failed.
            n300 |       0.15 |       fail |        TLE | n300 failed.
            n100 |        0.1 |       fail |        TLE | n100 failed.
            n200 |        0.1 |       fail |        TLE | n200 failed.
            n300 |        0.1 |       fail |        TLE | n300 failed.
        ```
