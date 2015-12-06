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
    { field: 'status',
        filter: {
            type: uiGridConstants.filter.SELECT,
            selectOptions: []
        }
    },
    { field: 'type',
        filter: {
            type: uiGridConstants.filter.SELECT,
            selectOptions: []
        }
    }];
    $scope.gridOptions = {
        enableSorting: true,
        enableFiltering: true,
        columnDefs: $scope.columns,
        data: [],
        onRegisterApi: function(gridApi){
            $scope.gridApi = gridApi;

            $scope.gridApi.core.on.rowsRendered($scope, filterTasks);
        }
    };

    function filterTasks() {
        var taskDict = $scope.dataService.taskDict;
        var filteredTasks = [];
        var filteredTaskDict = {};
        var rows = $scope.gridApi.grid.rows;
        var numRows = rows.length;
        for(var i = 0; i < numRows; i++) {
            var row = rows[i];
            if(row.visible)
            {
                var task = taskDict[row.entity.id];
                filteredTasks.push(task);
                filteredTaskDict[task.id] = task;
            }
        }

        $scope.dataService.filteredTasks = filteredTasks;
        $scope.dataService.filteredTaskDict = filteredTaskDict;
    };

    function fillColumFilterSelectOptions(options, columnDef) {
        var columnSelectOptions = [];

        for(var i = options.length-1; i >=0; i--)
        {
            var option = options[i];
            columnSelectOptions.push({ value: option, label: option })
        }

        columnDef.filter.selectOptions = columnSelectOptions;
    };

    $scope.$watch('dataService.taskstatustypes', function() {
        fillColumFilterSelectOptions($scope.dataService.taskstatustypes, $scope.columns[3]);
    });

    $scope.$watch('dataService.tasktypes', function() {
        fillColumFilterSelectOptions($scope.dataService.tasktypes, $scope.columns[4]);
    });
}
]);
