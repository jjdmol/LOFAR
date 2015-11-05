// $Id: controller.js 32761 2015-11-02 11:50:21Z schaap $

var gridControllerMod = angular.module('GridControllerMod', ['ui.grid']);

gridControllerMod.controller('GridController', ['$scope', 'dataService', 'uiGridConstants', function($scope, dataService, uiGridConstants) {

    $scope.dataService = dataService;

    $scope.$watch('dataService.tasks', function() {
        if('tasks' in $scope.dataService && $scope.dataService.tasks.length > 0)
            $scope.gridOptions.data = $scope.dataService.tasks;
        else
            $scope.gridOptions.data = []
    }, true);

    $scope.columns = [
    { field: 'name' },
    { field: 'from' },
    { field: 'to' },
    { field: 'status' },
    { field: 'type',
        filter: {
            type: uiGridConstants.filter.SELECT,
            selectOptions: [
            { value: '', label: 'All' },
            { value: 'Observation', label: 'Observation' },
            { value: 'Pipeline', label: 'Pipeline' } ]
        }
    }];
    $scope.gridOptions = {
        enableSorting: true,
        enableFiltering: true,
        columnDefs: $scope.columns,
        data: []
    };
}
]);
