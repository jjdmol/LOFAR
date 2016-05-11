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
        cellTemplate:'<a target="_blank" href="{{row.grid.appScope.dataService.config.mom_base_url}}/user/project/setUpMom2ObjectDetails.do?view=generalinfo&mom2ObjectId={{row.entity.project_mom2object_id}}">{{row.entity[col.field]}}</a>',
        width: '15%',
        filter: {
            type: uiGridConstants.filter.SELECT,
            selectOptions: []
        }
    },
    { field: 'mom_id',
        displayName: 'MoM ID',
        enableCellEdit: false,
        cellTemplate:'<a target="_blank" href="{{row.grid.appScope.dataService.config.mom_base_url}}/user/project/setUpMom2ObjectDetails.do?view=generalinfo&mom2ObjectId={{row.entity.mom2object_id}}">{{row.entity[col.field]}}</a>',
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
        type: 'date',
        enableCellEdit: false,
        enableCellEditOnFocus: false,
        cellTemplate:'<div style=\'text-align:left\'>{{row.entity[col.field] | date:\'yyyy-MM-dd HH:mm\'}}</div>'
//         editableCellTemplate: '<div><form name="inputForm"><div ui-grid-edit-datepicker row-field="MODEL_COL_FIELD" ng-class="\'colt\' + col.uid"></div></form></div>'
    },
    { field: 'endtime',
        displayName: 'End',
        width: '15%',
        type: 'date',
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
        enableCellEdit: false,
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
        if('tasks' in $scope.dataService && $scope.dataService.tasks.length > 0) {
            var tasks = [];
            for(var task of $scope.dataService.tasks) {
                var gridTask = {
                    id: task.id,
                    name: task.name,
                    project_name: task.project_name,
                    mom_id: task.mom_id,
                    otdb_id: task.otdb_id,
                    starttime: task.starttime,
                    endtime: task.endtime,
                    status: task.status,
                    type: task.type,
                    project_mom2object_id: task.project_mom2object_id,
                    mom2object_id: task.mom2object_id
                };
                tasks.push(gridTask);
            }

            $scope.gridOptions.data = tasks;
        } else
            $scope.gridOptions.data = []

        fillProjectsColumFilterSelectOptions();
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

    function fillProjectsColumFilterSelectOptions() {
        var projectNames = [];
        var momProjectsDict = $scope.dataService.momProjectsDict;
        var tasks = $scope.dataService.tasks;
        //get unique projectIds from tasks
        var task_project_ids = tasks.map(function(t) { return t.project_mom_id; });
        task_project_ids = task_project_ids.filter(function(value, index, arr) { return arr.indexOf(value) == index;})

        for(var project_id of task_project_ids) {
            if(momProjectsDict.hasOwnProperty(project_id)) {
                var projectName = momProjectsDict[project_id].name;
                if(!(projectName in projectNames)) {
                    projectNames.push(projectName);
                }
            }
        }
        projectNames.sort();
        fillColumFilterSelectOptions(projectNames, $scope.columns[1]);
    };

    $scope.$watch('dataService.momProjectsDict', fillProjectsColumFilterSelectOptions);

    $scope.$watch('dataService.selected_task_id', function() {
        var taskIdx = $scope.gridOptions.data.findIndex(function(row) {return row.id == dataService.selected_task_id});

        if(taskIdx > -1) {
            $scope.gridApi.selection.selectRow($scope.gridOptions.data[taskIdx]);
            $scope.gridApi.core.scrollTo($scope.gridOptions.data[taskIdx], null);
        }
    });
}
]);
