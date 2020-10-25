DROP TABLE IF EXISTS Customer;
CREATE TABLE Customer (
    code CHAR(3) NOT NULL, 
    name VARCHAR(32) NOT NULL DEFAULT '', 
    PRIMARY KEY (code)
) engine=innodb;

DROP TABLE IF EXISTS Product;
CREATE TABLE Product (
    code CHAR(3) NOT NULL, 
    name VARCHAR(32) NOT NULL DEFAULT '', 
    PRIMARY KEY (code)
) engine=innodb;

DROP TABLE IF EXISTS AuditLog;
CREATE TABLE AuditLog (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    customer CHAR(3) NOT NULL,
    product CHAR(3) NOT NULL,
    device_id BINARY(16) NOT NULL DEFAULT 0, 
    raised DATETIME,
    received DATETIME,
    CONSTRAINT FK_AuditLog_customer FOREIGN KEY (customer) REFERENCES Customer (code), 
    CONSTRAINT FK_AuditLog_product FOREIGN KEY (product) REFERENCES Product (code)
) engine=innodb;


CREATE OR REPLACE VIEW AuditLogHuman AS
SELECT id, customer, product, 
    device_id, 
    INSERT(INSERT(INSERT(INSERT(HEX(device_id),9,0,'-'),14,0,'-'),19,0,'-'),24,0,'-') AS hex_device_id,
    raised, received
FROM AuditLog;



DELIMITER //

CREATE OR REPLACE PROCEDURE TruncateAuditLog ()
 BEGIN
    SET @auditlog_date = CONCAT("AuditLog_", CURDATE()+0);
    SET @create_query = CONCAT("CREATE TABLE ", @auditlog_date, " LIKE AuditLog;");
    SET @insert_query = CONCAT("INSERT INTO ", @auditlog_date, " SELECT * FROM AuditLog;");
    SET @delete_query = CONCAT("DELETE AuditLog.* FROM AuditLog INNER JOIN ", @auditlog_date, " ON AuditLog.id=", @auditlog_date, ".id;");

    PREPARE create_stmt FROM @create_query; 
    EXECUTE create_stmt; 
    DEALLOCATE PREPARE create_stmt; 

    START TRANSACTION;
        PREPARE insert_stmt FROM @insert_query; 
        EXECUTE insert_stmt; 
        DEALLOCATE PREPARE insert_stmt; 

        PREPARE delete_stmt FROM @delete_query; 
        EXECUTE delete_stmt; 
        DEALLOCATE PREPARE delete_stmt; 
    COMMIT;
    
 END;
//

DELIMITER ;

CREATE OR REPLACE EVENT TruncateAuditLogJob
ON SCHEDULE EVERY 1 DAY STARTS TIMESTAMP(CONCAT(CURDATE(), ' 00:0:00'))
DO
    CALL TruncateAuditLog;

