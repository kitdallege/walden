import React from 'react';
import {
      List
    , Datagrid
    , Show
    , SimpleShowLayout
    , TextField
    , Create
    , SimpleForm
    , TextInput
    , Edit
    , DisabledInput
    , EditButton
} from 'react-admin';


export const TaxonomyList = (props) => (
    <List {...props} title="Taxonomy">
        <Datagrid>
            <TextField source="rowId" label="Id" />
            <TextField source="name" label="Application"/>
            <EditButton/>
        </Datagrid>
    </List>
);

export const TaxonomyEdit = (props) => (
    <Edit {...props}>
        <SimpleForm>
            <DisabledInput label="Id" source="rowId" />
            <TextInput source="name" />
        </SimpleForm>
    </Edit>
);
