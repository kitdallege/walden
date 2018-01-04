import React from 'react';
import {
      List
    , Datagrid
    , TextField
    , EditButton
    , Edit
    , SimpleForm
    , TextInput
    , DisabledInput
} from 'react-admin';


export const ApplicationList = (props) => (
    <List {...props} title="Applications">
        <Datagrid>
            <TextField source="id" />
            <TextField source="name" />
            <TextField source="description" />
            <EditButton />
        </Datagrid>
    </List>
);

export const ApplicationEdit = (props) => (
    <Edit {...props}>
        <SimpleForm>
            <DisabledInput label="Id" source="id" />
            <TextInput source="name" />
            <TextInput source="description" />
        </SimpleForm>
    </Edit>
);
