CREATE ROLE monitor_role;
CREATE USER monitor;
CREATE USER test;
GRANT monitor_role TO monitor;

CREATE TABLE tmp (id int, data text);

SET pg_block_systemcatalog.allow_role = '';

-- superuser
SELECT CURRENT_USER;

SELECT relname FROM pg_class WHERE relname = 'tmp';
SELECT table_name FROM information_schema.tables WHERE table_name = 'tmp';
SELECT relname FROM pg_stat_user_tables WHERE relname = 'tmp';

-- test(error)
SET ROLE test;
SELECT CURRENT_USER;

SELECT relname FROM pg_class WHERE relname = 'tmp';
SELECT table_name FROM information_schema.tables WHERE table_name = 'tmp';
SELECT relname FROM pg_stat_user_tables WHERE relname = 'tmp';

RESET ROLE;

-- monitor(error)
SET ROLE monitor;

SELECT CURRENT_USER;

SELECT relname FROM pg_class WHERE relname = 'tmp';
SELECT table_name FROM information_schema.tables WHERE table_name = 'tmp';
SELECT relname FROM pg_stat_user_tables WHERE relname = 'tmp';

RESET ROLE;

SET pg_block_systemcatalog.allow_role = 'monitor';

-- monitor(success)
SET ROLE monitor;

SELECT CURRENT_USER;

SELECT relname FROM pg_class WHERE relname = 'tmp';
SELECT table_name FROM information_schema.tables WHERE table_name = 'tmp';
SELECT relname FROM pg_stat_user_tables WHERE relname = 'tmp';

RESET ROLE;

DROP TABLE tmp;
REVOKE monitor_role FROM monitor; 
DROP ROLE test;
DROP ROLE monitor;
DROP ROLE monitor_role;
