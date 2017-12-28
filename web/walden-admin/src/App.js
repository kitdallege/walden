import React, { Component } from 'react';
import gql from 'graphql-tag';
// import buildApolloClient from 'aor-graphql-client';
import buildApolloClient from 'aor-graphql-client-graphcool';
import { Admin, Resource } from 'admin-on-rest';
import darkBaseTheme from 'material-ui/styles/baseThemes/darkBaseTheme';
import getMuiTheme from 'material-ui/styles/getMuiTheme';
import { EntityList } from './entities';
import { __schema as schema } from './schema';
import pluralize from 'pluralize';
import {
    CREATE,
    GET_LIST,
    GET_ONE,
    GET_MANY,
    GET_MANY_REFERENCE,
    UPDATE,
    DELETE,
    QUERY_TYPES,
} from 'aor-graphql-client/lib/constants';

const introspectionOptions = {
  schema,
  operationNames: {
        [GET_LIST]: resource => `all${pluralize(resource.name)}`,
        [GET_ONE]: resource => `${resource.name.toLowerCase()}`,
        [GET_MANY]: resource => `all${pluralize(resource.name)}`,
        [GET_MANY_REFERENCE]: resource => `all${pluralize(resource.name)}`,
        [CREATE]: resource => `create${resource.name}`,
        [UPDATE]: resource => `update${resource.name}`,
        [DELETE]: resource => `delete${resource.name}`,
    }
};
//const client = new ApolloClient();
//import { ApolloClient, createNetworkInterface } from 'apollo-client';

// const client = new ApolloClient({
//     networkInterface: createNetworkInterface('https://api.graph.cool/simple/v1/cj2kl5gbc8w7a0130p3n4eg78'),
// });
const buildFieldList = (introspectionResults, resource, aorFetchType) => {
    debugger
}

const queryBuilder = introspectionResults => (aorFetchType, resourceName, params) => {
    const resource = introspectionResults.resources.find(r => r.type.name === resourceName);
    switch (aorFetchType) {
        case 'GET_LIST':
            //${buildFieldList(introspectionResults, resource, aorFetchType)}
            return {
                query: gql`query ${resource[aorFetchType].name} {
                    data: ${resource[aorFetchType].name}(last: 20) {
                        items: nodes {
                            id
                            name
                            description
                            __typename
                        }
                        totalCount
                    }
                }`,
                variables: params, // params = { id: ... }
                parseResponse: response => ({'data': response.data.data.items, 'total': response.data.data.totalCount})
            }
            break;
        // ... other
    }
}

class App extends Component {
    constructor() {
        super();
        this.state = {restClient: null};
    }
    componentDidMount() {
        buildApolloClient({introspection: introspectionOptions, client:{uri:'http://0.0.0.0:5000/graphql'},  queryBuilder: queryBuilder})
            .then(restClient => this.setState({restClient}));
    }
    render() {
        const { restClient } = this.state;

        if (!restClient) {
            return <div>Loading</div>;
        }

        return (
            <Admin
                restClient={restClient}
                title="Walden Admin"
                theme={getMuiTheme(darkBaseTheme)}>
                <Resource name="Entity" list={EntityList}/>
            </Admin>
        );
    }
}

export default App;
