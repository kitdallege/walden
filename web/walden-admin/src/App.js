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
const theme = createMuiTheme({palette: {type: 'dark'}});

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
            // resolveIntrospection: function () {debugger}
        }).then(dataProvider => this.setState({dataProvider}));
    }
    render() {
        const { dataProvider } = this.state;
        if (!dataProvider) {
            return <div>Loading</div>;
        }
        return (
            <Admin
                dataProvider={dataProvider}
                title="Walden Admin"
                theme={theme}>
                <Resource
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
