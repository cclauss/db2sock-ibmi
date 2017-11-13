DROP PROCEDURE DB2JSON.DB2PROCJ;
DROP PROCEDURE DB2JSON.DB2PROCJR;
DROP PROCEDURE DB2JSON.DB2PROCJH;

CREATE PROCEDURE DB2JSON.DB2PROCJ(
INOUT JSONBUF CLOB(15M)) 
LANGUAGE C NOT DETERMINISTIC 
MODIFIES SQL DATA 
EXTERNAL NAME 'DB2JSON/DB2PROCJ(iJson400)' 
PARAMETER STYLE GENERAL;

CREATE PROCEDURE DB2JSON.DB2PROCJR(
IN JSONBUF CLOB(15M)) 
LANGUAGE RPGLE NOT DETERMINISTIC 
MODIFIES SQL DATA 
Result Sets 1 
EXTERNAL NAME 'DB2JSON/DB2PROCJR(iJson400R)' 
PARAMETER STYLE GENERAL;

CREATE PROCEDURE DB2JSON.DB2PROCJH(
IN JSONBUF CLOB(15M)) 
LANGUAGE RPGLE NOT DETERMINISTIC 
MODIFIES SQL DATA 
Result Sets 1 
EXTERNAL NAME 'DB2JSON/DB2PROCJR(iJson400H)' 
PARAMETER STYLE GENERAL;


