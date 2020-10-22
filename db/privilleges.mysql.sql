CREATE USER IF NOT EXISTS 'isosec_admin'@'localhost' IDENTIFIED BY 'kik9Aez0';
GRANT ALL ON * TO 'isosec_admin'@'localhost';

CREATE USER IF NOT EXISTS 'isosec_api'@'localhost' IDENTIFIED BY 'isosec-default';
GRANT SELECT ON * TO 'isosec_api'@'localhost';
GRANT INSERT ON AuditLog TO 'isosec_api'@'localhost';

