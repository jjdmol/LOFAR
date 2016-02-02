// $Id: controller.js 32761 2015-11-02 11:50:21Z schaap $

var gridControllerMod = angular.module('GridControllerMod', ['ui.grid',
                                                             'ui.grid.edit',
                                                             'ui.grid.cellNav',
                                                             'ui.grid.resizeColumns'/*,
                                                             'ui.grid.datepicker'*/]);

gridControllerMod.controller('GridController', ['$scope', 'dataService', 'uiGridConstants', function($scope, dataService, uiGridConstants) {

    $scope.dataService = dataService;

    $scope.$watch('dataService.tasks', function() {
        if('tasks' in $scope.dataService && $scope.dataService.tasks.length > 0)
            $scope.gridOptions.data = $scope.dataService.tasks;
        else
            $scope.gridOptions.data = []
    }, true);

    $scope.columns = [
    { field: 'name',
        enableCellEdit: false,
        width: '*'
    },
    { field: 'projectName',
        displayName:'Project',
        enableCellEdit: false,
        width: '*',
        filter: {
            type: uiGridConstants.filter.SELECT,
            selectOptions: []
        }
    },
    { field: 'starttime',
        width: '*',
        enableCellEditOnFocus: true,
        cellTemplate:'<div style=\'text-align:right\'>{{row.entity[col.field] | date:\'yyyy-MM-dd HH:mm\'}}</div>'
//         editableCellTemplate: '<div><form name="inputForm"><div ui-grid-edit-datepicker row-field="MODEL_COL_FIELD" ng-class="\'colt\' + col.uid"></div></form></div>'
    },
    { field: 'endtime', enableCellEdit: true,
        width: '*',
        enableCellEditOnFocus: true,
        cellTemplate:'<div style=\'text-align:right\'>{{row.entity[col.field] | date:\'yyyy-MM-dd HH:mm\'}}</div>'
    },
    { field: 'status',
        enableCellEdit: false,
        width: '*',
        filter: {
            type: uiGridConstants.filter.SELECT,
            selectOptions: []
        }
    },
    { field: 'type',
        enableCellEdit: false,
        width: '*',
        filter: {
            type: uiGridConstants.filter.SELECT,
            selectOptions: []
        }
    }];
    $scope.gridOptions = {
        enableGridMenu: false,
        enableSorting: true,
        enableFiltering: true,
        enableColumnResize: true,
        gridMenuShowHideColumns: false,
        columnDefs: $scope.columns,
        data: [],
        onRegisterApi: function(gridApi){
            $scope.gridApi = gridApi;

            $scope.gridApi.core.on.rowsRendered($scope, filterTasks);

            gridApi.edit.on.afterCellEdit($scope,function(rowEntity, colDef, newValue, oldValue){
                console.log('edited row id:' + rowEntity.id + ' Column:' + colDef.name + ' newValue:' + newValue + ' oldValue:' + oldValue);

                var task = $scope.dataService.taskDict[rowEntity.id];
                $scope.dataService.putTask(task);
            });
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

        for(var i = 0; i < options.length; i++)
        {
            var option = options[i];
            columnSelectOptions.push({ value: option, label: option })
        }

        columnDef.filter.selectOptions = columnSelectOptions;
    };

    $scope.$watch('dataService.taskstatustypes', function() {
        fillColumFilterSelectOptions($scope.dataService.taskstatustypes, $scope.columns[4]);
    });

    $scope.$watch('dataService.tasktypes', function() {
        fillColumFilterSelectOptions($scope.dataService.tasktypes, $scope.columns[5]);
    });

    $scope.$watch('dataService.momProjectsDict', function() {
        var projectNames = [];
        var momProjectsDict = $scope.dataService.momProjectsDict;
        for(var key in momProjectsDict) {
            if(momProjectsDict.hasOwnProperty(key)) {
                var projectName = momProjectsDict[key].name;
                if(!(projectName in projectNames)) {
                    projectNames.push(projectName);
                }
            }
        }
        projectNames.sort();
        fillColumFilterSelectOptions(projectNames, $scope.columns[1]);
    });
}
]);
