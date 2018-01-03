import { TypeKind } from 'graphql';
import gql from 'graphql-tag';
// import { __schema as schema } from '../schema';
import pluralize from 'pluralize';
import {
    CREATE,
    GET_LIST,
    GET_ONE,
    GET_MANY,
    GET_MANY_REFERENCE,
    UPDATE,
    DELETE
} from 'react-admin';


// TODO: UPDATE CACHED SCHEMA ALL THE TIME. It works with introspection
// but it def adds a chunk of time as its a heavy pull.
export const introspectionOptions = {
    //schema,
    operationNames: {
       [GET_LIST]: resource => `all${pluralize(resource.name)}`,
       [GET_ONE]: resource  => `get${resource.name}`,
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
export const isSubObject = field => {
    return (
        field.type.kind === TypeKind.OBJECT ||
        (field.type.kind === TypeKind.LIST &&
            (field.type.ofType.kind === TypeKind.OBJECT || field.type.ofType.kind === TypeKind.LIST)) ||
        (field.type.kind === TypeKind.NON_NULL &&
            (field.type.ofType.kind === TypeKind.OBJECT || field.type.ofType.kind === TypeKind.LIST))
    );
};

const getType = field => {
    if (field.type.kind === TypeKind.LIST || field.type.kind === TypeKind.NON_NULL) {
        return field.type.ofType;
    }
    return field.type;
};

const isResource = (field, resources) => {
    const type = getType(field);
    //return resources.some(({ name }) => name === type.name);
    return resources.some(r => r.type.name === type.name);
};

const buildFieldList = (introspectionResults, resource, aorFetchType, depth=0) => {
    const types = introspectionResults.types;
    const resources = introspectionResults.resources;
    return resource.type.fields
        .filter(field => !field.name.startsWith('__'))
        .map(field => {
            try {
                if (isSubObject(field, types) && depth < 4) {
                    let typeToCheck = getType(field);
                    if (isResource(field, resources)) {
                        const type = types.find(t => t.name === typeToCheck.name);
                        const resource = introspectionResults.resources.find(r => r.type.name === type.name);
                        if (type && resource) {
                            const subFields = buildFieldList(
                                introspectionResults,
                                resource,
                                aorFetchType,
                                depth + 1
                            );
                            return `${field.name} { ${subFields} }`;
                        }
                    }
                    return false;
                }
                return field.name;
            } catch (err) {
                debugger
            }
            return false;
        })
        .filter(f => f !== false)
        .join(' ');
}

export const queryBuilder = introspectionResults => (aorFetchType, resourceName, params) => {
    console.debug(introspectionResults);
    const resource = introspectionResults.resources.find(r => r.type.name === resourceName);
    var result = {};
    switch (aorFetchType) {
        case 'GET_LIST':
        case 'GET_MANY':
        case 'GET_MANY_REFERENCE':
            result =  {
                query: gql`query ${resource[aorFetchType].name} {
                    data: ${resource[aorFetchType].name}(first: 20) {
                        items: nodes {
                            ${buildFieldList(introspectionResults, resource, aorFetchType)}
                        }
                        totalCount
                    }
                }`,
                variables: params, // params = { id: ... }
                parseResponse: response => ({'data': response.data.data.items, 'total': response.data.data.totalCount})
            };
            console.info(result);
            break;
        case 'GET_ONE':
            result = {
                query: gql`query ${resource[aorFetchType].name}($id: Int) {
                    data: ${resource[aorFetchType].name}(arg0: $id) {
                        ${buildFieldList(introspectionResults, resource, aorFetchType)}
                    }
                }`,
                variables: {id: params.id},
                parseResponse: response => ({data:response.data.data, id:response.data.data.id})
            }
            break;
        case 'CREATE':
        case 'DELETE':
        case 'UPDATE':
            break;
        default:
            return undefined;
    }
    return result;
}


export const buildQueryFactory = function (introspectionResults, otherOptions) {
    // debugger
    return queryBuilder(introspectionResults)
}
