// $Id: controller.js 32761 2015-11-02 11:50:21Z schaap $

var gridControllerMod = angular.module('GridControllerMod', ['ui.grid', 'ui.grid.treeView']);

gridControllerMod.controller('GridController', ['$scope', 'dataService', 'uiGridTreeViewConstants', function($scope, dataService, uiGridTreeViewConstants) {

    $scope.dataService = dataService;

    $scope.$watch('dataService.tasks', function() {
        if('tasks' in $scope.dataService && $scope.dataService.tasks.length > 0)
        {
            var taskList = [];
            for ( var i = 0; i < $scope.dataService.tasks.length; i++ ){
                var node = $scope.dataService.tasks[i];
                node.$$treeLevel = 0;
                taskList.push(node);
                var nodeTasks = node['tasks'];
                for ( var j = 0; j < nodeTasks.length; j++ ){
                    nodeTasks[j].$$treeLevel = 1;
                    taskList.push(nodeTasks[j]);
                }
            }
            $scope.gridOptions.data = taskList;
        }
        else
            $scope.gridOptions.data = []
    }, true);

    $scope.columns = [{ field: 'name' }, { field: 'from' }, { field: 'to' }];
    $scope.gridOptions = {
        enableSorting: true,
        enableFiltering: true,
        columnDefs: $scope.columns,
        showTreeExpandNoChildren: false,
        data: []
    };
}
]);
