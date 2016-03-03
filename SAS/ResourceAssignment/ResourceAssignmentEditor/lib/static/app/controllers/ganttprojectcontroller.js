// $Id: ganttprojectcontroller.js 32761 2015-11-02 11:50:21Z schaap $

var ganttProjectControllerMod = angular.module('GanttProjectControllerMod', [
                                        'gantt',
                                        'gantt.sortable',
                                        'gantt.movable',
                                        'gantt.drawtask',
                                        'gantt.tooltips',
                                        'gantt.bounds',
                                        'gantt.progress',
                                        'gantt.table',
                                        'gantt.tree',
                                        'gantt.groups',
                                        'gantt.overlap',
                                        'gantt.resizeSensor']).config(['$compileProvider', function($compileProvider) {
    $compileProvider.debugInfoEnabled(false); // Remove debug info (angularJS >= 1.3)
}]);

ganttProjectControllerMod.controller('GanttProjectController', ['$scope', 'dataService', function($scope, dataService) {

    var self = this;
    self.doInitialCollapse = true;

    $scope.dataService = dataService;
    $scope.ganttData = []

    self.taskStatusColors = {'prepared':'#aaff00',
                             'approved':'#ffaa00',
                             'on_hold':'#ff0000',
                             'conflict':'#ffccaa',
                             'prescheduled': '#6666ff',
                             'scheduled': '#ff66ff',
                             'queued': '#bb6644',
                             'active': '#77ff77',
                             'completing': '#776688',
                             'finished': '#66ff33',
                             'aborted': '#ff3366',
                             'error': '#ff4488',
                             'obsolete': '#555555'}

    $scope.options = {
        mode: 'custom',
        scale: 'day',
        sideMode: 'Tree',
        columns: ['model.name', 'starttime', 'endtime'],
        treeTableColumns: ['starttime', 'endtime'],
        columnsHeaders: {'model.name' : 'Name', 'starttime': 'From', 'endtime': 'To'},
        columnsClasses: {'model.name' : 'gantt-column-name', 'starttime': 'gantt-column-from', 'endtime': 'gantt-column-to'},
        columnsFormatters: {
            'starttime': function(starttime) {
                return starttime !== undefined ? starttime.format('lll') : undefined;
            },
            'endtime': function(endtime) {
                return endtime !== undefined ? endtime.format('lll') : undefined;
            }
        },
        autoExpand: 'both',
        api: function(api) {
            // API Object is used to control methods and events from angular-gantt.
            $scope.api = api;

            api.core.on.ready($scope, function () {
                    api.tasks.on.moveEnd($scope, moveHandler);
                    api.tasks.on.resizeEnd($scope, moveHandler);
                }
            );
        }
    };

    function moveHandler(item)
    {
        var task_id = item.model.id;

        if(task_id) {
            var task = $scope.dataService.taskDict[task_id];
            var updatedTask = {
                id: task.id,
                starttime: item.model.from._d.toISOString(),
                endtime: item.model.to._d.toISOString()
            };
            $scope.dataService.putTask(updatedTask);
        }
    };

    function updateGanttData() {

        var projectsDict = $scope.dataService.momProjectsDict;
        var numProjecs = $scope.dataService.momProjects.length;

        var taskDict = $scope.dataService.filteredTaskDict;
        var tasks = $scope.dataService.filteredTasks;
        var numTasks = tasks.length;

        var tasktypesDict = $scope.dataService.tasktypesDict;
        var tasktypes = $scope.dataService.tasktypes;
        var numTasktypes = tasktypes.length;

        var ganntRowsDict = {};

        if(numProjecs > 0 && numTasks > 0 && numTasktypes > 0) {
            for(var i = 0; i < numTasks; i++) {
                var task = tasks[i];

                var projectRowId = 'project_' + task.project_mom_id;
                var ganntProjectRow = ganntRowsDict[projectRowId];

                if(!ganntProjectRow) {
                    var project = projectsDict[task.project_mom_id];

                    if(project) {
                        ganntProjectRow = {
                            'id': projectRowId,
                            'name': project.name,
                            'tasks': []
                        };

                        ganntRowsDict[projectRowId] = ganntProjectRow;
                    }
                }

                if(ganntProjectRow) {
                    var typeRowId = 'project_' + task.project_mom_id + '_type_' + task.type_id;
                    var ganntTypeRow = ganntRowsDict[typeRowId];

                    if(!ganntTypeRow) {
                        var tasktype = tasktypesDict[task.type_id].name;

                        if(tasktype) {
                            ganntTypeRow = {
                                'id': typeRowId,
                                'parent': projectRowId,
                                'name': tasktype,
                                'tasks': []
                            };

                            ganntRowsDict[typeRowId] = ganntTypeRow;
                        }
                    }

                    if(ganntTypeRow) {
                        var rowTask = {
                            id: task.id.toString(),
                            name: task.name,
                            'from': task.starttime,
                            'to': task.endtime,
                            'color': self.taskStatusColors[task.status]
                        };

                        ganntTypeRow.tasks.push(rowTask);
                    }
                }
            }
        }

        var ganntRows = [];

        for (var rowId in ganntRowsDict)
            ganntRows.push(ganntRowsDict[rowId]);

        ganntRows.sort(function(a, b) { return ((a.name < b.name) ? -1 : ((a.name > b.name) ? 1 : 0)); });
        $scope.ganttData = ganntRows;
    };

    $scope.$watch('dataService.filteredTasks', updateGanttData, true);
    $scope.$watch('dataService.tasktypes', updateGanttData, true);
    $scope.$watch('dataService.momProjectsDict', updateGanttData, true);
}
]);
