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
    , ReferenceInput
    , SelectInput
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

export const TaxonList = (props) => (
    <List {...props} title="Taxons">
        <Datagrid>
            <TextField source="rowId" label="Id" />
            <TextField source="name" label="Taxon"/>
            <TextField source="parentPath" label="Path"/>
            <EditButton/>
        </Datagrid>
    </List>
);
export const TaxonCreate = (props) => (
    <Create {...props}>
        <SimpleForm>
            <TextInput source="name" />
            <TextInput source="parentPath" label="Path"/>
            <ReferenceInput label="Page" source="pageId" reference="Page" allowEmpty>
                <SelectInput optionText="name" optionValue="rowId"/>
            </ReferenceInput>
            <ReferenceInput label="Resource" source="resourceId" reference="Resource" allowEmpty>
                <SelectInput optionText="name" optionValue="rowId"/>
            </ReferenceInput>
        </SimpleForm>
    </Create>
);
export const TaxonEdit = (props) => (
    <Edit {...props}>
        <SimpleForm>
            <DisabledInput source="rowId" label="Id" />
            <DisabledInput source="name" />
            <DisabledInput source="parentPath" label="Path"/>
            <ReferenceInput label="Page" source="pageId" reference="Page" allowEmpty>
                <SelectInput optionText="name" optionValue="rowId"/>
            </ReferenceInput>
            <ReferenceInput label="Resource" source="resourceId" reference="Resource" allowEmpty>
                <SelectInput optionText="name" optionValue="rowId"/>
            </ReferenceInput>
        </SimpleForm>
    </Edit>
);
