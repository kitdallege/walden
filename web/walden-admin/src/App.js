import React, { Component } from 'react';
import buildApolloClient from 'aor-graphql-client';
// import buildApolloClient from 'aor-graphql-client-graphcool';
import { Admin, Resource } from 'admin-on-rest';
import darkBaseTheme from 'material-ui/styles/baseThemes/darkBaseTheme';
import getMuiTheme from 'material-ui/styles/getMuiTheme';
// walden
import { EntityList } from './entities';
import { introspectionOptions, queryBuilder} from './client';

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
                <Resource name="Entity" list={EntityList} />
            </Admin>
        );
    }
}

export default App;
