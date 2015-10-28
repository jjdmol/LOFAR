// $Id$
angular.module('raeApp').controller('DataController', ['$http', function($http) {
    var store = this;
    this.tasks = [];

    function getTasks() {
        $http.get('/rest/tasks').success(function(result) {
            store.tasks = result.tasks;
        });
    };

    getTasks();
}]);
