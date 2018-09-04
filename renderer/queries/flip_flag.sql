update resource set date_updated = default, dirty = false where id = ANY($1);
