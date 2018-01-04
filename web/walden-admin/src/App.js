import React, { Component } from 'react';
import { Admin, Resource } from 'react-admin';
import buildGraphQLProvider from 'ra-data-graphql';
import { createMuiTheme } from 'material-ui/styles';
// walden
import {
      EntityCreate
    , EntityList
    , EntityShow
    , EntityEdit
    , AttributeList
} from './entities';
import { ApplicationList, ApplicationEdit } from './applications';
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
                    name="WaldenApplication"
                    options={{ label: "Applications"}}
                    list={ApplicationList}
                    edit={ApplicationEdit}
                    />
                <Resource
                    name="WaldenEntity"
                    options={{ label: "Entities"}}
                    create={EntityCreate}
                    list={EntityList} show={EntityShow}
                    edit={EntityEdit}/>
                <Resource
                    name="WaldenAttribute"
                    options={{ label: "Walden Attributes"}}
                    list={AttributeList} />
            </Admin>
        );
    }
}

export default App;
