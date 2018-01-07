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
            <TextField source="name" label="Name / Website"/>
            <EditButton/>
        </Datagrid>
    </List>
);

export const TaxonomyShow = (props) => (
    <Show {...props}>
        <SimpleShowLayout>
            <TextField source="rowId" label="Id"/>
            <TextField source="name" />
        </SimpleShowLayout>
    </Show>
);
// <ReferenceInput label="Application" source="application_id" reference="WaldenApplication" allowEmpty>
//         <SelectInput optionText="name"/>
// </ReferenceInput>
export const TaxonomyCreate = (props) => (
        <Create {...props}>
            <SimpleForm>
                <TextInput source="name" />
            </SimpleForm>
        </Create>
);

export const TaxonomyEdit = (props) => (
    <Edit {...props}>
        <SimpleForm>
            <DisabledInput source="rowId" label="Id" />
            <TextInput source="name" />
        </SimpleForm>
    </Edit>
);
