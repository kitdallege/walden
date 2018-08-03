select p.id, p.name || '.html' as filename,
	replace(p.parent_path::text, '.', '/') as path,
	spec.template, spec.query, p.query_params
from page as p join page_spec as spec on spec.id = p.page_spec_id 
where p.dirty = true and p.page_spec_id = $1 and p.id > $2 
order by p.page_spec_id, p.taxon_id, p.id 
limit 2000;
