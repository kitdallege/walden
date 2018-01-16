/* The Walden equivalent of django settings/urls modules for a project.
 * Each 'site/project' typically has a sql file assoicated with it which
 * contains the data definitions needed to create a site.
 */
SET search_path TO walden;

DO $$
DECLARE
    entity_id   entity.id%TYPE := 1;
    org_id      organization.id%TYPE;
    site_id     site.id%TYPE;
    taxonomy_id taxonomy.id%TYPE;
    res_id      resource.id%TYPE;
    page_id     page.id%TYPE;
BEGIN
    -- create user to admin the site.
    INSERT INTO walden_user (username, first_name, last_name, email, password)
        VALUES ('demo', '', '', 'demo@email.com', 'demo');
    -- create org
    SELECT * INTO STRICT org_id
        FROM walden_create_organization('Demo Co.');
    -- create site
    SELECT * INTO STRICT site_id
        FROM walden_create_site(org_id, 'Demo Site', 'demo.walden.com');
    -- set site.settings
    -- create taxonomy-for-site
    INSERT INTO taxonomy (site_id, name)
        VALUES (site_id, 'Demo Taxonomy')
        ON CONFLICT (name) DO NOTHING
        RETURNING id INTO taxonomy_id;
    -- create static page resource.
    INSERT INTO resource (name, type, entity_id)
        VALUES ('Static Page', 'STATIC', entity_id)
        ON CONFLICT (name) DO NOTHING
        RETURNING id INTO res_id;
    -- create a page template to use
    INSERT INTO page (name, title)
        VALUES ('base', 'Demo Site')
        ON CONFLICT (name) DO NOTHING
        RETURNING id INTO page_id;
    -- add homepage taxon
    INSERT INTO taxon(taxonomy_id, name, parent_path, resource_id, page_id)
        VALUES (taxonomy_id, 'Home', 'root', res_id, page_id)
        ON CONFLICT (parent_path) DO NOTHING;
    -- render hompage ?
END$$ LANGUAGE plpgsql;
