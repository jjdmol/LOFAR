// $Id: $
angular.module('raeApp').controller('DataController', ['$http', function($http) {
    var store = this;
    this.data = [];

    $http.get('/rest/data.json').success(function(data) {
        store.data = data.data;
    });
}]);
