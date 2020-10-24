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
