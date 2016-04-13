// $Id: controller.js 32761 2015-11-02 11:50:21Z schaap $

var gridControllerMod = angular.module('GridControllerMod', ['ui.grid',
                                                             'ui.grid.edit',
                                                             'ui.grid.selection',
                                                             'ui.grid.cellNav',
                                                             'ui.grid.resizeColumns']);

gridControllerMod.controller('GridController', ['$scope', 'dataService', 'uiGridConstants', function($scope, dataService, uiGridConstants) {

    $scope.dataService = dataService;

    $scope.columns = [
    { field: 'name',
        enableCellEdit: false,
        width: '15%'
    },
    { field: 'project_name',
        displayName:'Project',
        enableCellEdit: false,
        cellTemplate:'<a target="_blank" href="https://lofar.astron.nl/mom3/user/project/setUpMom2ObjectDetails.do?view=generalinfo&mom2Id={{row.entity.project_mom_id}}">{{row.entity[col.field]}}</a>',
        width: '15%',
        filter: {
            type: uiGridConstants.filter.SELECT,
            selectOptions: []
        }
    },
    { field: 'mom_id',
        displayName: 'MoM ID',
        enableCellEdit: false,
        cellTemplate:'<a target="_blank" href="https://lofar.astron.nl/mom3/user/project/setUpMom2ObjectDetails.do?view=generalinfo&mom2Id={{row.entity[col.field]}}">{{row.entity[col.field]}}</a>',
        width: '7.5%'
    },
    { field: 'otdb_id',
        displayName: 'SAS ID',
        enableCellEdit: false,
        width: '7.5%'
    },
    { field: 'starttime',
        displayName: 'Start',
        width: '15%',
        enableCellEdit: false,
        enableCellEditOnFocus: false,
        cellTemplate:'<div style=\'text-align:left\'>{{row.entity[col.field] | date:\'yyyy-MM-dd HH:mm\'}}</div>'
//         editableCellTemplate: '<div><form name="inputForm"><div ui-grid-edit-datepicker row-field="MODEL_COL_FIELD" ng-class="\'colt\' + col.uid"></div></form></div>'
    },
    { field: 'endtime',
        displayName: 'End',
        width: '15%',
        enableCellEdit: false,
        enableCellEditOnFocus: false,
        cellTemplate:'<div style=\'text-align:left\'>{{row.entity[col.field] | date:\'yyyy-MM-dd HH:mm\'}}</div>'
    },
    { field: 'status',
        enableCellEdit: true,
        width: '12.5%',
        filter: {
            type: uiGridConstants.filter.SELECT,
            selectOptions: []
        },
        editableCellTemplate: 'ui-grid/dropdownEditor',
        editDropdownOptionsArray: []
    },
    { field: 'type',
        enableCellEdit: false,
        width: '12.5%',
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
        enableRowSelection: true,
        enableRowHeaderSelection: true,
        enableFullRowSelection: false,
        enableSelectionBatchEvent:false,
        multiSelect:false,
        gridMenuShowHideColumns: false,
        columnDefs: $scope.columns,
        data: [],
        onRegisterApi: function(gridApi){
            $scope.gridApi = gridApi;

            $scope.gridApi.core.on.rowsRendered($scope, filterTasks);

            gridApi.edit.on.afterCellEdit($scope,function(rowEntity, colDef, newValue, oldValue){
                var task = $scope.dataService.taskDict[rowEntity.id];
                var newTask = { id: task.id, status: task.status };
                $scope.dataService.putTask(newTask);
            });

            gridApi.selection.on.rowSelectionChanged($scope,function(row){
                if(row.entity.id && row.isSelected) {
                    $scope.dataService.selected_task_id = row.entity.id;
                }
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

        if(options) {
            for(var i = 0; i < options.length; i++)
            {
                var option = options[i];
                columnSelectOptions.push({ value: option, label: option })
            }
        }

        columnDef.filter.selectOptions = columnSelectOptions;
    };

    $scope.$watch('dataService.tasks', function() {
        if('tasks' in $scope.dataService && $scope.dataService.tasks.length > 0)
            $scope.gridOptions.data = $scope.dataService.tasks;
        else
            $scope.gridOptions.data = []
    }, true);

    $scope.$watch('dataService.taskstatustypes', function() {
        taskstatustypenames = $scope.dataService.taskstatustypes.map(function(x) { return x.name; });
        fillColumFilterSelectOptions(taskstatustypenames, $scope.columns[6]);
        $scope.columns[6].editDropdownOptionsArray = $scope.dataService.taskstatustypes.map(function(x) { return {id:x.name, value:x.name}; });
    });

    $scope.$watch('dataService.tasktypes', function() {
        tasktypenames = $scope.dataService.tasktypes.map(function(x) { return x.name; });
        fillColumFilterSelectOptions(tasktypenames, $scope.columns[7]);
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

    $scope.$watch('dataService.selected_task_id', function() {
        var taskIdx = $scope.gridOptions.data.findIndex(function(row) {return row.id == dataService.selected_task_id});

        if(taskIdx > -1) {
            $scope.gridApi.selection.selectRow($scope.gridOptions.data[taskIdx]);
        }
    });
}
]);
