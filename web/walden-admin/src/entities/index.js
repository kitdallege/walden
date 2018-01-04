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
            <TextField source="id" />
            <TextField source="application.name" label="Application"/>
            <TextField source="name" label="Entity"/>
            <TextField source="description" />
            <EditButton/>
        </Datagrid>
    </List>
);

export const EntityShow = (props) => (
    <Show {...props}>
        <SimpleShowLayout>
            <TextField source="id" />
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
            <TextInput source="name" />
            <TextInput source="description" />
        </SimpleForm>
    </Edit>
);


export const AttributeList = (props) => (
    <List {...props} title="Walden Attributes">
        <Datagrid>
            <TextField source="id" />
            <TextField source="name" />
            <TextField source="description" />
            <TextField source="attributeType.name" label="Type"/>
            <EditButton />
        </Datagrid>
    </List>
);
