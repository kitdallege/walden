import React, { Component } from 'react';
import buildGraphQLProvider from 'ra-data-graphql';
import { Admin, Resource } from 'react-admin';
// import darkBaseTheme from 'material-ui/styles/baseThemes/darkBaseTheme';
// import getMuiTheme from 'material-ui/styles/getMuiTheme';
import { createMuiTheme } from 'material-ui/styles';
// walden
import { EntityList } from './entities';
import { introspectionOptions, buildQueryFactory} from './client';
import {
    CREATE,
    GET_LIST,
    GET_ONE,
    GET_MANY,
    GET_MANY_REFERENCE,
    UPDATE,
    DELETE,
    QUERY_TYPES,
} from 'react-admin';


const theme = createMuiTheme({
  palette: {
    type: 'dark', // Switching the dark mode on is a single property value change.
  },
});

class App extends Component {
    constructor() {
        super();
        this.state = { dataProvider: null };
    }
    componentDidMount() {
        buildGraphQLProvider({
            introspection: introspectionOptions,
            client:{
                uri:'http://0.0.0.0:5000/graphql'
            },
            buildQuery: buildQueryFactory,
            // resolveIntrospection: function (client, options) {
            //     debugger
            //     console.info(options.operationNames[GET_ONE]("Entity"));
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
                <Resource name="Entity" list={EntityList} />
            </Admin>
        );
    }
}

export default App;
