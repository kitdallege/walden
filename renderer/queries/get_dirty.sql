select
        r.id,
        coalesce(nullif(r.name, ''), 'index') || '.html' as filename,
        replace(r.mount::text, '.', '/') as path,
        replace(replace(t.parent_path::text, '.', '/'), 'root', '')  || t.name as template,
        replace(replace(q.parent_path::text, '.', '/'), 'root', '')  || q.name as query,
        r.view_args
from resource as r
join resource_view as rview on rview.id = r.view_id
join template as t on t.id = rview.template_id
join query as q on q.id = rview.query_id
where r.dirty = true and r.view_id = $1 and r.id > $2
order by r.view_id, r.id
limit 2000;
