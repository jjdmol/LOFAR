// $Id: controller.js 32761 2015-11-02 11:50:21Z schaap $

var gridControllerMod = angular.module('GridControllerMod', ['DataControllerMod']);

gridControllerMod.controller('GridController', ['$scope', 'dataService', function($scope, dataService) {

    $scope.dataService = dataService;

    $scope.$watch('dataService.tasks', function() {
        if('tasks' in $scope.dataService && $scope.dataService.tasks.length > 0)
            $scope.gridOptions.data = $scope.dataService.tasks[0]['tasks'];
        else
            $scope.gridOptions.data = []
    }, true);

    $scope.columns = [{ field: 'name' }, { field: 'from' }, { field: 'to' }];
    $scope.gridOptions = {
        enableSorting: true,
        columnDefs: $scope.columns,
        data: []
    };
}
]);
