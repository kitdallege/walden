import React from 'react';
import { List, Datagrid, TextField } from 'react-admin';

export const EntityList = (props) => (
        <List {...props}>
            <Datagrid>
                <TextField source="id" />
                <TextField source="name" />
                <TextField source="description" />
            </Datagrid>
        </List>
);
