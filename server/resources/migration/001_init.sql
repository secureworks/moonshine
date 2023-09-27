CREATE TABLE implant (
    id                        VARCHAR,
    listener                  VARCHAR,
    name                      VARCHAR UNIQUE,
    public_key                VARCHAR,
    architecture              INTEGER DEFAULT 0,
    operating_system          INTEGER DEFAULT 0,
    process_id                INTEGER DEFAULT 0,
    process_user              VARCHAR DEFAULT '',
    process_path              VARCHAR DEFAULT '',
    system_name               VARCHAR DEFAULT '',
    system_addrs              VARCHAR DEFAULT '',
    UNIQUE(id)
);

CREATE TABLE listener (
    id                        VARCHAR,
    status                    INTEGER DEFAULT 0,
    name                      VARCHAR UNIQUE,
    package                   VARCHAR,
    implant_package           VARCHAR,
    implant_connection_string VARCHAR,
    UNIQUE(id)
);

CREATE TABLE service (
    id                        VARCHAR,
    status                    INTEGER DEFAULT 0,
    name                      VARCHAR UNIQUE,
    package                   VARCHAR,
    UNIQUE(id)
);

CREATE TABLE argument (
    owner                     VARCHAR,
    position                  INTEGER DEFAULT 0,
    data                      VARCHAR,
    UNIQUE(owner, data)
);

CREATE TABLE library (
    owner                     VARCHAR,
    type                      INTEGER DEFAULT 1,
    name                      VARCHAR,
    data                      VARCHAR,
    UNIQUE(owner, type, name)
);

CREATE TABLE task (
    id                        VARCHAR,
    implant                   VARCHAR,
    status                    INTEGER DEFAULT 0,
    script                    VARCHAR DEFAULT '',
    success                   INTEGER DEFAULT -1,
    output                    VARCHAR DEFAULT '',
    UNIQUE(id),
    FOREIGN KEY(implant) REFERENCES implant(id)
);

CREATE TABLE job (
    id                        VARCHAR,
    name                      VARCHAR,
    implant                   VARCHAR,
    status                    INTEGER DEFAULT 0,
    package                   VARCHAR DEFAULT '',
    success                   INTEGER DEFAULT -1,
    output                    VARCHAR DEFAULT '',
    UNIQUE(id),
    FOREIGN KEY(implant) REFERENCES implant(id)
);

CREATE TABLE config (
    key                       VARCHAR NOT NULL PRIMARY KEY,
    value                     VARCHAR
);

CREATE TRIGGER ImplantAutoGenID
AFTER INSERT ON implant
FOR EACH ROW
WHEN (NEW.id IS NULL)
BEGIN
   UPDATE implant SET id = (select lower(hex(randomblob(5)))) WHERE rowid = NEW.rowid;
END;

CREATE TRIGGER ListenerAutoGenID
AFTER INSERT ON listener
FOR EACH ROW
WHEN (NEW.id IS NULL)
BEGIN
   UPDATE listener SET id = (select lower(hex(randomblob(5)))) WHERE rowid = NEW.rowid;
END;

CREATE TRIGGER ServiceAutoGenID
AFTER INSERT ON service
FOR EACH ROW
WHEN (NEW.id IS NULL)
BEGIN
   UPDATE service SET id = (select lower(hex(randomblob(5)))) WHERE rowid = NEW.rowid;
END;

CREATE TRIGGER TaskAutoGenID
AFTER INSERT ON task
FOR EACH ROW
WHEN (NEW.id IS NULL)
BEGIN
   UPDATE task SET id = (select lower(hex(randomblob(5)))) WHERE rowid = NEW.rowid;
END;

CREATE TRIGGER JobAutoGenID
AFTER INSERT ON job
FOR EACH ROW
WHEN (NEW.id IS NULL)
BEGIN
   UPDATE job SET id = (select lower(hex(randomblob(5)))) WHERE rowid = NEW.rowid;
END;
