DROP SCHEMA blackboard CASCADE;
CREATE SCHEMA blackboard;

\i create_blackboard_tables.sql;
\i create_blackboard_functions.sql;
\i create_blackboard_triggers.sql;
