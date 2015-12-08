# K-SkyBand-Query-in-PostgreSQL
incomplete k-skyband, constrained skyline, and group-by skyline queries on incomplete data in postgresql

## Algorithm Discription
### kISB algorithm:
* Please see Mr. Gao's paper: [Processing k-skyband, constrained skyline, and group-by skyline queries on incomplete data] (http://www.armourcy.com/skyband.pdf)

## Testing Environment
  * Operation System: Mac OSX / Ubuntu 14.04
  * PostgreSQL version: PostgreSQL 9.4.4

## How to use?
### 0. Install postgre environment fist

for ubuntu:

~~~terminal
    sudo apt-get install postgresql-9.4
    sudo apt-get install postgresql-server-dev-9.4
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

in terminal you will see a contrib path in output like this:
"/usr/local/Cellar/postgresql/9.4.4/share/postgresql/contrib"

### 3. Import skyband function (in postgresql)

1. Login in postgre with the database that you want to add function to

  (if you want to add function to **all** database, then you should login in **template1** database,
google it for more details.)

2. Use postgresql command to import function like below

  (remember to replace content in '[]' with the path that you got in step 2.)

~~~sql
    \i [/usr/local/Cellar/postgresql/9.4.4/share/postgresql/contrib/]skyband.sql
~~~

3. You will see two "CREATE FUNCTION" outputed if nothong wrong

### 4. Performing skyband query with warehouse algorithem
if we have such a table:
~~~sql
postgres=> select * from hotel;

 id |     name     | price | distance | noisy | star
----+--------------+-------+----------+-------+------
  1 | cs hotel     |   200 |    30.62 |     5 |    3
  2 | c hotel      |   400 |       20 |     6 |    3
  3 | c++ hotel    |   300 |       20 |       |    3
  4 | java hotel   |   330 |     17.5 |       |    3
  5 | python hotel |   278 |          |     2 |    4
  6 | ruby hotel   |   399 |          |       |    4
  7 | obj-c hotel  |       |          |     4 |    4
  8 | swift hotel  |       |    25.99 |     9 |
 35 | pascal hotel |   243 |          |     7 |    3
 36 | lol hotel    |     1 |        1 |     1 |    5
(10 rows)
~~~

then we can run command like below using skyband function:

~~~sql
-- skyline query (k = 1)
postgres=> select * from skyband('select name, price, distance, noisy from hotel', 1) as (name text, price int, distance real, noisy int);
   name    | price | distance | noisy
-----------+-------+----------+-------
 lol hotel |     1 |        1 |     1
(1 row)

-- k-skyband query (k > 1)
postgres=> select * from skyband('select name, price, distance, noisy from hotel', 3) as (name text, price int, distance real, noisy int);
     name     | price | distance | noisy
--------------+-------+----------+-------
 cs hotel     |   200 |    30.62 |     5
 lol hotel    |     1 |        1 |     1
 python hotel |   278 |          |     2
 obj-c hotel  |       |          |     4
(5 rows)

-- constrained skyband query 
postgres=> select * from skyband('select name, price, distance, noisy from hotel', 3) as (name text, price int, distance real, noisy int) where price between 100 and 280;
     name     | price | distance | noisy
--------------+-------+----------+-------
 cs hotel     |   200 |    30.62 |     5
 python hotel |   278 |          |     2
(2 rows)
~~~

Note that skyband function needs two parameters:

1. the selection clause, can not be NULL (text)
2. k (integer)

(ps: you should also provide the output format using as clause, otherwise it won't work).


There is another function call skyand_ext which is more powerfull and can use to solve group by skyband query.

~~~sql
-- skyline query (k = 1)
postgres=> select * from skyband_ext('name, price, distance, noisy', 'hotel', NULL, 1, 'name text, price int, distance real, noisy int') as (name text, price int, distance real, noisy int);
   name    | price | distance | noisy
-----------+-------+----------+-------
 lol hotel |     1 |        1 |     1
(1 row)

-- k-skyband query (k > 1)
postgres=> select * from skyband_ext('name, price, distance, noisy', 'hotel', NULL, 3, 'name text, price int, distance real, noisy int') as (name text, price int, distance real, noisy int);
     name     | price | distance | noisy
--------------+-------+----------+-------
 cs hotel     |   200 |    30.62 |     5
 lol hotel    |     1 |        1 |     1
 python hotel |   278 |          |     2
 obj-c hotel  |       |          |     4
(4 rows)

-- constrained skyband query 
postgres=> select * from skyband_ext('name, price, distance, noisy', 'hotel', NULL, 3, 'name text, price int, distance real, noisy int') as (name text, price int, distance real, noisy int) where price between 100 and 280;
     name     | price | distance | noisy
--------------+-------+----------+-------
 cs hotel     |   200 |    30.62 |     5
 python hotel |   278 |          |     2
(2 rows)

-- groupby skyband query (grouped by star attribute)
postgres=> select * from skyband_ext('name, price, distance, noisy, star', 'hotel', 'star', 1, 'name text, price int, distance real, noisy int, star int') as (name text, price int, distance real, noisy int, star int);
     name     | price | distance | noisy | star
--------------+-------+----------+-------+------
 swift hotel  |       |    25.99 |     9 |
 cs hotel     |   200 |    30.62 |     5 |    3
 python hotel |   278 |          |     2 |    4
 lol hotel    |     1 |        1 |     1 |    5
(4 rows)

~~~

Note that skyband_ext functin takes 5 parameters:

1. the fields that you want to use, can not be NULL (text)
2. the name of the table, can not be NULL (text)
3. the attribute that you want to use as groupby standard, can be NULL (text)
4. k (integer)
5. inner output format, can not be NULL and must match the number of fields in parameter 1. (text)

(ps: you should also provide the outer output format using as clause, otherwise it won't work).

(pps: here the inner output format actully is redundent, but I'm still not found a good way to hide it, in most cases, you can fill both inner and outer format with the original names and types in your table)

## Contact us
1. If you have any question about this paper, you can contact Mr. Gao: gaoyj@zju.edu.cn
2. The projet is coded by Armour Guo, feel free to ask any questions: armourcy@gmail.com

