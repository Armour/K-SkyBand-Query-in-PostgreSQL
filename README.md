# K-SkyBand-Query-in-PostgreSQL
incomplete k-skyband, constrained skyline, and group-by skyline queries on incomplete data in postgresql

## Algorithm Discription
### kISB algorithm:
* Please see Mr. Gao's paper: [Processing k-skyband, constrained skyline, and group-by skyline queries on incomplete data] (http://www.armourcy.com/skyband.pdf)

## Testing Environment
  * Operation System: Mac OSX / Ubuntu 14.04
  * PostgreSQL version: PostgreSQL 9.4.4

## How to use?
### 0. Install postgresql-server-dev fist

for ubuntu:

~~~terminal
    sudo apt-get install postgresql-server-dev-all
    sudo apt-get install postgresql-contrib-9.4
~~~

for mac:

~~~terminal
    brew install postgresql
~~~

### 1. Clone and enter my repo (in terminal)
~~~terminal
    git clone git@github.com:Armour/K-SkyBand-Query-in-PostgreSQL.git
    cd K-SkyBand-Query-in-PostgreSQL
~~~

### 2. Make and install skyband function (in terminal)
~~~terminal
    cd skyband
    make
    sudo make install
~~~

in the terminal you will see one path in output like this:
"/usr/local/Cellar/postgresql/9.4.4/share/postgresql/contrib"

### 3. Import skyband function (in postgresql)

use postgresql command to enter below command, remember to replace content in '[]' with the path that you get in step 2.

~~~sql
    \i [/usr/local/Cellar/postgresql/9.4.4/share/postgresql/contrib/]skyband.sql
~~~

### 4. Performing skyband query with warehouse algorithem
if we have such a table:
~~~sql
postgres=# select * from hotel;

 id |     name     | price | distance | noisy
----+--------------+-------+----------+-------
  1 | cs hotel     |   200 |       30 |     5
  2 | c hotel      |   400 |       20 |     6
  3 | c++ hotel    |   300 |       20 |
  4 | java hotel   |   330 |       17 |
  5 | python hotel |   278 |          |     2
  6 | ruby hotel   |   399 |          |
  7 | obj-c hotel  |       |          |     4
  8 | swift hotel  |       |       25 |     9
 35 | pascal hotel |   243 |          |     7
 36 | lol hotel    |     1 |        1 |     1
(10 rows)
~~~

then we can run command like below
~~~sql
    select * from skyband('select price, distance, noisy from hotel', 2) as (price real, distance real, noisy real);
~~~

### 5. Here's the result
~~~sql
 price | distance | noisy
-------+----------+-------
   200 |       30 |     5
     1 |        1 |     1
   278 |          |     2
       |          |     4
(4 rows)
~~~

## Contact us
1. If you have any question about this paper, you can contact Mr. Gao: gaoyj@zju.edu.cn
2. The projet is coded by Armour Guo, feel free to ask any questions: gc497052684@gmail.com

