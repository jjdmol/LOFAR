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
                                        'gantt.dependencies',
                                        'gantt.overlap',
                                        'gantt.resizeSensor']).config(['$compileProvider', function($compileProvider) {
    $compileProvider.debugInfoEnabled(false); // Remove debug info (angularJS >= 1.3)
}]);

ganttProjectControllerMod.controller('GanttProjectController', ['$scope', 'dataService', function($scope, dataService) {

    var self = this;
    self.doInitialCollapse = true;

    $scope.dataService = dataService;
    $scope.ganttData = [];

    self.taskStatusColors = dataService.taskStatusColors;

    $scope.options = {
        mode: 'custom',
        viewScale: '1 hours',
        currentDate: 'line',
        currentDateValue: $scope.dataService.lofarTime,
        columnMagnet: '1 minutes',
        timeFramesMagnet: false,
        sideMode: 'Tree',
        autoExpand: 'both',
        taskOutOfRange: 'truncate',
        dependencies: true,
        api: function(api) {
            // API Object is used to control methods and events from angular-gantt.
            $scope.api = api;

            api.core.on.ready($scope, function () {
                    api.tasks.on.moveEnd($scope, moveHandler);
                    api.tasks.on.resizeEnd($scope, moveHandler);
                }
            );

            api.directives.on.new($scope, function(directiveName, directiveScope, element) {
                if (directiveName === 'ganttRow' || directiveName === 'ganttRowLabel' ) {
                    element.bind('click', function(event) {
                        if(directiveScope.row.model.project) {
                            $scope.dataService.selected_project_id = directiveScope.row.model.project.id;
                        }
                    });
                } else if (directiveName === 'ganttTask') {
                    element.bind('click', function(event) {
                        if(directiveScope.task.model.raTask) {
                            $scope.dataService.selected_task_id = directiveScope.task.model.raTask.id;
                        }
                    });
                }
            });

            api.directives.on.destroy($scope, function(directiveName, directiveScope, element) {
                if (directiveName === 'ganttRow' || directiveName === 'ganttRowLabel' || directiveName === 'ganttTask') {
                    element.unbind('click');
                }
            });
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
        if(!dataService.initialLoadComplete) {
            return;
        }

        var projectsDict = $scope.dataService.momProjectsDict;
        var numProjecs = $scope.dataService.momProjects.length;

        var taskDict = $scope.dataService.filteredTaskDict;
        var tasks = $scope.dataService.filteredTasks;
        var numTasks = tasks.length;

        var tasktypesDict = $scope.dataService.tasktypesDict;
        var tasktypes = $scope.dataService.tasktypes;
        var numTasktypes = tasktypes.length;

        if(numProjecs == 0 || numTasktypes == 0){
            $scope.ganttData = [];
            return;
        }

        var editableTaskStatusIds = $scope.dataService.editableTaskStatusIds;

        var ganntRowsDict = {};

        if(numProjecs > 0 && numTasks > 0 && numTasktypes > 0) {
            $scope.options.fromDate = $scope.dataService.taskTimes.minStarttime;
            $scope.options.toDate = $scope.dataService.taskTimes.maxEndtime;
            var fullTimespanInMinutes = $scope.dataService.taskTimes.fullTimespanInMinutes;

            if(fullTimespanInMinutes > 14*24*60) {
                $scope.options.viewScale = '1 days';
            } else if(fullTimespanInMinutes > 7*24*60) {
                $scope.options.viewScale = '6 hours';
            } else if(fullTimespanInMinutes > 2*24*60) {
                $scope.options.viewScale = '3 hours';
            } else {
                $scope.options.viewScale = '1 hours';
            }

            for(var i = 0; i < numTasks; i++) {
                var task = tasks[i];

                var projectRowId = 'project_' + task.project_mom_id;
                var ganntProjectRow = ganntRowsDict[projectRowId];

                if(!ganntProjectRow) {
                    var project = projectsDict[task.project_mom_id];

                    if(project) {
                        ganntProjectRow = {
                            id: projectRowId,
                            name: project.name,
                            project: project,
                            tasks: []
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
                                id: typeRowId,
                                parent: projectRowId,
                                name: tasktype,
                                project: project,
                                tasks: []
                            };

                            ganntRowsDict[typeRowId] = ganntTypeRow;
                        }
                    }

                    if(ganntTypeRow) {
                        var rowTask = {
                            id: task.id.toString(),
                            name: task.name,
                            from: task.starttime,
                            to: task.endtime,
                            raTask: task,
                            color: self.taskStatusColors[task.status],
                            movable: $.inArray(task.status_id, editableTaskStatusIds) > -1
                        };

                        if(task.predecessor_ids && task.predecessor_ids.length > 0) {
                            rowTask['dependencies'] = [];
                            for(var predId of task.predecessor_ids) {
                                 rowTask['dependencies'].push({'from': predId});
                            }
                        }

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

    $scope.$watch('dataService.initialLoadComplete', updateGanttData);
    $scope.$watch('dataService.tasks', updateGanttData, true);
    $scope.$watch('dataService.resources', updateGanttData);
    $scope.$watch('dataService.resourceClaims', updateGanttData, true);
    $scope.$watch('dataService.resourceGroups', updateGanttData);
    $scope.$watch('dataService.resourceGroupMemberships', updateGanttData);
    $scope.$watch('dataService.filteredTaskDict', updateGanttData);
    $scope.$watch('dataService.momProjectsDict', updateGanttData, true);
    $scope.$watch('dataService.taskTimes', updateGanttData, true);
    $scope.$watch('dataService.lofarTime', function() {$scope.options.currentDateValue= $scope.dataService.lofarTime;});
}
]);
