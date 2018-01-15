import React, { Component } from 'react';
import { Admin, Resource, Delete } from 'react-admin';
import buildGraphQLProvider from 'ra-data-graphql';
import { createMuiTheme } from 'material-ui/styles';
// walden
import {
      EntityCreate
    , EntityList
    //, EntityShow // Currently Broken for whatever reason.
    , EntityEdit
} from './entities';
import BuildIcon from 'material-ui-icons/Build'; // ? this seems wrong to me.
import {
      TaxonomyList
    , TaxonomyEdit
    , TaxonomyCreate
    , TaxonomyShow
    , TaxonList
    , TaxonCreate
    , TaxonEdit
} from './taxonomy';
import { introspectionOptions, buildQueryFactory} from './client';
import customRoutes from './customRoutes';
import { introspectionQuery } from 'graphql';
import gql from 'graphql-tag';
const theme = createMuiTheme({palette: {type: 'dark'}});

class MyResource extends Resource {

};

class App extends Component {
    constructor() {
        super();
        this.state = { dataProvider: null };
    }
    componentDidMount() {
        buildGraphQLProvider({
            introspection: introspectionOptions,
            client:{uri:'http://0.0.0.0:5000/graphql'},
            buildQuery: buildQueryFactory,
            _resolveIntrospection: async function (client, options) {
                debugger
                const schema = options.schema
                    ? options.schema
                    : await client
                          .query({ query: gql`${introspectionQuery}` })
                          .then(({ data: { __schema } }) => __schema);
                debugger
            }
        }).then(dataProvider => this.setState({dataProvider}));
    }
    render() {
        const { dataProvider } = this.state;
        if (!dataProvider) {
            return <div>Loading</div>;
        }
        // TODO: introspect
        debugger
        return (
            <Admin
                dataProvider={dataProvider}
                title="Walden Admin"
                theme={theme}
                customRoutes={customRoutes}>
                <MyResource
                    name="Entity"
                    icon={BuildIcon}
                    create={EntityCreate}
                    list={EntityList}
                    /* show={EntityShow} */
                    edit={EntityEdit}
                    remove={Delete}/>
                <Resource name="EntityTaxon" />
                <Resource
                    name="Taxonomy"
                    create={TaxonomyCreate}
                    list={TaxonomyList} show={TaxonomyShow}
                    edit={TaxonomyEdit}
                    remove={Delete}/>
                <Resource
                    name="Taxon"
                    create={TaxonCreate}
                    list={TaxonList}
                    edit={TaxonEdit} />
                <Resource name="Page" />
                <Resource name="Resource" />
                <Resource name="Application" />
                <Resource name="Asset" />
                <Resource name="Config" />
                <Resource name="WaldenUser" label="User" />
                <Resource name="Widget" />
                <Resource name="WidgetOnPage" />
                <Resource name="Query" />
                <Resource name="QueryEntityRef" />
            </Admin>
        );
    }
}

export default App;
