import React, { Component } from 'react';
import { Admin, Resource } from 'react-admin';
import buildGraphQLProvider from 'ra-data-graphql';
import { createMuiTheme } from 'material-ui/styles';
// walden
import { EntityList, EntityShow } from './entities';
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
            buildQuery: buildQueryFactory
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
                <Resource name="Entity" list={EntityList} show={EntityShow}/>
            </Admin>
        );
    }
}

export default App;
