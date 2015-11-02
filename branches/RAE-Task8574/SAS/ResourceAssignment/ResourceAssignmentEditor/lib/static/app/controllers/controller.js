// $Id$
angular.module('raeApp').controller('DataController', ['$http', '$scope', function($http, $scope) {
    var store = this;
    this.tasks = [];
    this.gtasks = [];

    function getTasks() {
        $http.get('/rest/tasks').success(function(result) {
//             for(var key in result.data) {
//                 var elem = result.tasks[key];
//                 elem.from = new Date(elem.from);
//                 elem.to = new Date(elem.to);
//             }

            store.tasks = result.tasks;
            store.gtasks = result.tasks;
        });
    };

    getTasks();

    this.test = [
    {name: 'row1', tasks: [
        {name: 'task1', from: new Date(2013, 10, 26, 8, 0, 0), to: new Date(2013, 10, 26, 8, 0, 0)},
        {name: 'task2', from: new Date(2013, 11, 26, 8, 0, 0), to: new Date(2013, 11, 26, 9, 0, 0)}
    ]
    },
    {name: 'row2', tasks: [
        {name: 'task3', from: new Date(2013, 10, 26, 8, 0, 0), to: new Date(2013, 13, 26, 8, 0, 0)}
    ]
    }];
  }
]);
