select
        p.id,
        coalesce(nullif(p.name, ''), 'index') || '.html' as filename,
        replace(p.mount::text, '.', '/') as path,
        replace(replace(t.parent_path::text, '.', '/'), 'root', '')  || t.name as template,
        replace(replace(q.parent_path::text, '.', '/'), 'root', '')  || q.name as query,
        p.query_args
from page as p
join page_spec as spec on spec.id = p.page_spec_id
join template as t on t.id = spec.template_id
join query as q on q.id = spec.query_id
where p.dirty = true and p.page_spec_id = $1 and p.id > $2
order by p.page_spec_id, p.id
limit 2000;

