import React, { Component } from 'react';
import { Admin, Resource, Delete } from 'react-admin';
import buildGraphQLProvider from 'ra-data-graphql';
import { createMuiTheme } from 'material-ui/styles';
// walden
import {
      EntityCreate
    , EntityList
    //, EntityShow
    , EntityEdit
} from './entities';
import BuildIcon from 'material-ui-icons/Build'; // ? this seems wrong to me.
import {
      TaxonomyList
    , TaxonomyEdit
    , TaxonomyCreate
    , TaxonomyShow
} from './taxonomy';
import { introspectionOptions, buildQueryFactory} from './client';
// import {
//     CREATE,
//     GET_LIST,
//     GET_ONE,
//     GET_MANY,
//     GET_MANY_REFERENCE,
//     UPDATE,
//     DELETE,
//     QUERY_TYPES,
// } from 'react-admin';

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
            // resolveIntrospection: function () {
            //     debugger
            // }
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
                <Resource
                    name="Taxonomy"
                    create={TaxonomyCreate}
                    list={TaxonomyList} show={TaxonomyShow}
                    edit={TaxonomyEdit}
                    remove={Delete}/>
            </Admin>
        );
        /*
        <Resource
            name="Taxonomy"
            list={TaxonomyList}
            edit={TaxonomyEdit}
            />
        <Resource
            name="Taxon"
            list={TaxonList}
            edit={TaxonEdit}/>
        */
    }
}

export default App;
