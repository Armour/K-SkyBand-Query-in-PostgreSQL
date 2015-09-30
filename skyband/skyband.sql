CREATE OR REPLACE FUNCTION skyband(text, int)
    RETURNS SETOF record
    AS '$libdir/skyband', 'SkybandQuery'
    LANGUAGE C STRICT;
