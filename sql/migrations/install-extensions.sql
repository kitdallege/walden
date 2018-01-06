--
-- PostgreSQL database dump
--

-- Dumped from database version 9.6.3
-- Dumped by pg_dump version 9.6.3
SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;
SET row_security = off;

--
-- Name: plpgsql; Type: EXTENSION; Schema: -; Owner:
--

CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;


--
-- Name: EXTENSION plpgsql; Type: COMMENT; Schema: -; Owner:
--

COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';


--
-- Name: plpythonu; Type: EXTENSION; Schema: -; Owner:
--

CREATE EXTENSION IF NOT EXISTS plpythonu WITH SCHEMA pg_catalog;


--
-- Name: EXTENSION plpythonu; Type: COMMENT; Schema: -; Owner:
--

COMMENT ON EXTENSION plpythonu IS 'PL/PythonU untrusted procedural language';


--
-- PostgreSQL database dump complete
--

-- CREATE EXTENSION IF NOT EXISTS temporal_tables WITH SCHEMA walden;
--
-- COMMENT ON EXTENSION temporal_tables IS 'This extension provides support for temporal tables. System-period data versioning (also known as transaction time or system time) allows you to specify that old rows are archived into another table (that is called the history table).'

-- Walden
CREATE EXTENSION IF NOT EXISTS walden WITH SCHEMA walden CASCADE;
COMMENT ON EXTENSION walden IS 'This extension provides a web development platform within PostgreSQL';
