/* The Walden equivalent of django settings/urls modules for a project.
 * Each 'site/project' typically has a sql file assoicated with it which
 * contains the data definitions needed to create a site.
 */


DO $$
DECLARE
    site_id integer;
BEGIN
    -- create user to admin the site.
    INSERT INTO walden_user (username, first_name, last_name, email, password)
        VALUES ('demo', '', '', 'demo@email.com', 'demo');

    -- create org
    INSERT INTO organization (name) VALUES ('Demo Co.');
    site_id := SELECT walden_create_site('Demo Co.', 'Demo Site', 'demo.walden.com');

    -- set site.settings
    -- create taxon-for-site
    -- add homepage taxon
    -- wire up static page resource to homepage taxon.
    -- render hompage ?
END$$ LANGUAGE PLPGSQL VOLATILE;
