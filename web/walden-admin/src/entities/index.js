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

export const EntityList = (props) => (
    <List {...props} title="Entity's">
        <Datagrid>
            <TextField source="rowId" />
            <TextField source="schema" label="Application"/>
            <TextField source="name" label="Entity"/>
            <EditButton/>
        </Datagrid>
    </List>
);

export const EntityShow = (props) => (
    <Show {...props}>
        <SimpleShowLayout>
            <TextField source="rowId" />
            <TextField source="name" />
            <TextField source="description" />
        </SimpleShowLayout>
    </Show>
);
// <ReferenceInput label="Application" source="application_id" reference="WaldenApplication" allowEmpty>
//         <SelectInput optionText="name"/>
// </ReferenceInput>
export const EntityCreate = (props) => (
        <Create {...props}>
            <SimpleForm>
                <TextInput source="name" />
                <TextInput source="description" />
            </SimpleForm>
        </Create>
);

export const EntityEdit = (props) => (
    <Edit {...props}>
        <SimpleForm>
            <DisabledInput label="Id" source="id" />
            <TextInput source="schema" />
            <TextInput source="name" />
        </SimpleForm>
    </Edit>
);
