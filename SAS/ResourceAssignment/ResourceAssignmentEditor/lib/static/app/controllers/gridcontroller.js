// $Id: controller.js 32761 2015-11-02 11:50:21Z schaap $

var gridControllerMod = angular.module('GridControllerMod', ['ui.grid',
                                                             'ui.grid.edit',
                                                             'ui.grid.selection',
                                                             'ui.grid.cellNav',
                                                             'ui.grid.resizeColumns',
                                                             'ui.grid.autoResize']);

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
        cellTemplate:'<a target="_blank" href="https://lofar.astron.nl/mom3/user/project/setUpMom2ObjectDetails.do?view=generalinfo&mom2ObjectId={{row.entity.project_mom2object_id}}">{{row.entity[col.field]}}</a>',
        width: '15%',
        filter: {
            type: uiGridConstants.filter.SELECT,
            selectOptions: []
        }
    },
    { field: 'starttime',
        displayName: 'Start',
        width: '14%',
        type: 'date',
        enableCellEdit: false,
        enableCellEditOnFocus: false,
        cellTemplate:'<div style=\'text-align:left\'>{{row.entity[col.field] | date:\'yyyy-MM-dd HH:mm:ss\'}}</div>',
        sort: { direction: uiGridConstants.ASC }
    },
    { field: 'endtime',
        displayName: 'End',
        width: '14%',
        type: 'date',
        enableCellEdit: false,
        enableCellEditOnFocus: false,
        cellTemplate:'<div style=\'text-align:left\'>{{row.entity[col.field] | date:\'yyyy-MM-dd HH:mm:ss\'}}</div>'
    },
    { field: 'duration',
        displayName: 'Duration',
        width: '8%',
        enableFiltering: false,
        enableCellEdit: false,
        enableCellEditOnFocus: false,
        cellTemplate:'<div style=\'text-align:left\'>{{row.entity[col.field] | secondsToHHmmss}}</div>'
    },
    { field: 'status',
        enableCellEdit: true,
        width: '8%',
        filter: {
            type: uiGridConstants.filter.SELECT,
            selectOptions: []
        },
        editableCellTemplate: 'ui-grid/dropdownEditor',
        editDropdownOptionsArray: []
    },
    { field: 'type',
        enableCellEdit: false,
        width: '8%',
        filter: {
            type: uiGridConstants.filter.SELECT,
            selectOptions: []
        }
    },
    { field: 'mom_id',
        displayName: 'MoM ID',
        enableCellEdit: false,
        cellTemplate:'<a target="_blank" href="https://lofar.astron.nl/mom3/user/project/setUpMom2ObjectDetails.do?view=generalinfo&mom2ObjectId={{row.entity.mom2object_id}}">{{row.entity[col.field]}}</a>',
        width: '8%'
    },
    { field: 'otdb_id',
        displayName: 'SAS ID',
        enableCellEdit: false,
        width: '8%'
    }];
    $scope.gridOptions = {
        enableGridMenu: false,
        enableSorting: true,
        enableFiltering: true,
        enableCellEdit: false,
        enableColumnResize: true,
        enableHorizontalScrollbar: uiGridConstants.scrollbars.NEVER,
        enableRowSelection: true,
        enableRowHeaderSelection: true,
        enableFullRowSelection: false,
        enableSelectionBatchEvent:false,
        multiSelect:false,
        gridMenuShowHideColumns: false,
        columnDefs: $scope.columns,
        data: [],
//         rowTemplate: "<div ng-repeat=\"(colRenderIndex, col) in colContainer.renderedColumns track by col.uid\" ui-grid-one-bind-id-grid=\"rowRenderIndex + '-' + col.uid + '-cell'\" class=\"ui-grid-cell\" ng-class=\"{ 'ui-grid-row-header-cell': col.isRowHeader }\" role=\"{{col.isRowHeader ? 'rowheader' : 'gridcell'}}\" ui-grid-cell></div>"
        rowTemplate: "<div ng-repeat=\"(colRenderIndex, col) in colContainer.renderedColumns track by col.uid\" ui-grid-one-bind-id-grid=\"rowRenderIndex + '-' + col.uid + '-cell'\" class=\"ui-grid-cell\" ng-class=\"{ 'ui-grid-row-header-cell': col.isRowHeader }\" role=\"{{col.isRowHeader ? 'rowheader' : 'gridcell'}}\" ui-grid-cell context-menu>",
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

    function populateList() {
        if('tasks' in $scope.dataService && $scope.dataService.tasks.length > 0) {
            var viewFrom = $scope.dataService.viewTimeSpan.from;
            var viewTo = $scope.dataService.viewTimeSpan.to;

            var tasks = [];

            for(var task of $scope.dataService.tasks) {
                if(task.endtime >= viewFrom && task.starttime <= viewTo) {
                    var gridTask = {
                        id: task.id,
                        name: task.name,
                        project_name: task.project_name,
                        mom_id: task.mom_id,
                        otdb_id: task.otdb_id,
                        starttime: task.starttime,
                        endtime: task.endtime,
                        duration: task.duration,
                        status: task.status,
                        type: task.type,
                        project_mom2object_id: task.project_mom2object_id,
                        mom2object_id: task.mom2object_id
                    };
                    tasks.push(gridTask);
                }
            }

            $scope.gridOptions.data = tasks;
        } else
            $scope.gridOptions.data = []

        fillProjectsColumFilterSelectOptions();
    };

    function jumpToSelectedTaskRow() {
        var taskIdx = $scope.gridOptions.data.findIndex(function(row) {return row.id == dataService.selected_task_id});

        if(taskIdx > -1) {
            $scope.gridApi.selection.selectRow($scope.gridOptions.data[taskIdx]);
            $scope.gridApi.core.scrollTo($scope.gridOptions.data[taskIdx], null);
        }
    };

    $scope.$watch('dataService.taskChangeCntr', populateList);
    $scope.$watch('dataService.viewTimeSpan', function() {
        populateList();
        setTimeout(jumpToSelectedTaskRow, 250);
    }, true);

    $scope.$watch('dataService.taskstatustypes', function() {
        taskstatustypenames = $scope.dataService.taskstatustypes.map(function(x) { return x.name; });
        fillColumFilterSelectOptions(taskstatustypenames, $scope.columns[5]);
        $scope.columns[6].editDropdownOptionsArray = $scope.dataService.taskstatustypes.map(function(x) { return {id:x.name, value:x.name}; });
    });

    $scope.$watch('dataService.tasktypes', function() {
        tasktypenames = $scope.dataService.tasktypes.map(function(x) { return x.name; });
        fillColumFilterSelectOptions(tasktypenames, $scope.columns[6]);
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
    $scope.$watch('dataService.selected_task_id', jumpToSelectedTaskRow);}
]);

gridControllerMod.directive('contextMenu', ['$document', function($document) {
    return {
      restrict: 'A',
      scope: {
      },
      link: function($scope, $element, $attrs) {
        function handleContextMenuEvent(event) {
            //pragmatic 'hard-coded' way of getting the dataService and the rowEntity via scope tree.
            var dataService = $scope.$parent.$parent.$parent.$parent.$parent.$parent.$parent.$parent.dataService;
            var rowEntity = $scope.$parent.$parent.$parent.row.entity;

            if(!dataService || !rowEntity)
                return true;

            var taskId = rowEntity.id;
            var task = dataService.taskDict[taskId];
            dataService.selected_task_id = taskId;

            var docElement = angular.element($document);

            //search for already existing contextmenu element
            while($document.find('#grid-context-menu').length) {
                //found, remove it, so we can create a fresh one
                $document.find('#grid-context-menu')[0].remove();

                //unbind document close event handlers
                docElement.unbind('click', closeContextMenu);
                docElement.unbind('contextmenu', closeContextMenu);
            }

            //create contextmenu element
            //with list of menu items,
            //each with it's own action
            var contextmenuElement = angular.element('<div id="grid-context-menu"></div>');
            var ulElement = angular.element('<ul class="dropdown-menu" role="menu" style="left:' + event.clientX + 'px; top:' + event.clientY + 'px; z-index: 100000; display:block;"></ul>');
            contextmenuElement.append(ulElement);
            var liElement = angular.element('<li><a href="#">Copy Task</a></li>');
            ulElement.append(liElement);
            liElement.on('click', function() {
                dataService.copyTask(task);
                closeContextMenu();
            });

            var closeContextMenu = function(cme) {
                contextmenuElement.remove();

                //unbind document close event handlers
                docElement.unbind('click', closeContextMenu);
                docElement.unbind('contextmenu', closeContextMenu);
            };

            //click anywhere to remove the contextmenu
            docElement.bind('click', closeContextMenu);
            docElement.bind('contextmenu', closeContextMenu);

            //add contextmenu to body
            var body = $document.find('body');
            body.append(contextmenuElement);

            //prevent bubbling event upwards
            return false;
        }

        $element.bind('contextmenu', handleContextMenuEvent);

        $scope.$on('$destroy', function() {
            console.log('destroy');
            $element.unbind('contextmenu', handleContextMenuEvent);
        });
      }
    };
  }]);
