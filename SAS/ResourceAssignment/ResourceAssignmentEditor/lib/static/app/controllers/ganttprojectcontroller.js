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
                                        'gantt.resizeSensor',
                                        'gantt.contextmenu']).config(['$compileProvider', function($compileProvider) {
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

            api.directives.on.new($scope, function(directiveName, directiveScope, directiveElement) {
                if (directiveName === 'ganttRow' || directiveName === 'ganttRowLabel' ) {
                    directiveElement.bind('click', function(event) {
                        if(directiveScope.row.model.project) {
                            $scope.dataService.selected_project_id = directiveScope.row.model.project.id;
                        }
                    });
                } else if (directiveName === 'ganttTask') {
                    directiveElement.bind('click', function(event) {
                        if(directiveScope.task.model.raTask) {
                            $scope.dataService.selected_task_id = directiveScope.task.model.raTask.id;
                        }
                    });
                    directiveElement.bind('dblclick', function(event) {
                        if(directiveScope.task.model.raTask) {
                            $scope.dataService.selected_task_id = directiveScope.task.model.raTask.id;
                            $scope.jumpToSelectedTasks();
                        }
                    });
//                     directiveElement.bind('contextmenu', function(event) {
//                         if(directiveScope.task.model.raTask) {
//                             $scope.dataService.selected_task_id = directiveScope.task.model.raTask.id;
//                         }
//
//                         //search for already existing contextmenu element
//                         if(directiveElement.find('#gantt-project-context-menu').length) {
//                             //found, remove it, so we can create a fresh one
//                             directiveElement.find('#gantt-project-context-menu')[0].remove();
//                         }
//
//                         //create contextmenu element
//                         //with list of menu items,
//                         //each with it's own action
//                         var contextmenuElement = angular.element('<div id="gantt-project-context-menu"></div>');
//                         ulElement = angular.element('<ul style="z-index:10000; position:fixed; top:initial; left:initial; display:block;" role="menu" class="dropdown-menu"></ul>');
//                         contextmenuElement.append(ulElement);
//                         liElement = angular.element('<li><a href="#">Copy Task</a></li>');
//                         ulElement.append(liElement);
//                         liElement.on('click', function() {
//                             $scope.dataService.copyTask(directiveScope.task.model.raTask);
//                             closeContextMenu();
//                         });
//
//                         var closeContextMenu = function() {
//                             contextmenuElement.remove();
//                             angular.element(document).unbind('click', closeContextMenu);
//                         };
//
//                         //click anywhere to remove the contextmenu
//                         angular.element(document).bind('click', closeContextMenu);
//
//                         //add contextmenu to clicked element
//                         directiveElement.append(contextmenuElement);
//
//                         //prevent bubbling event upwards
//                         return false;
//                     });
                }
            });

            api.directives.on.destroy($scope, function(directiveName, directiveScope, directiveElement) {
                if (directiveName === 'ganttRow' || directiveName === 'ganttRowLabel' || directiveName === 'ganttTask') {
                    directiveElement.unbind('click');
                }
                if (directiveName === 'ganttTask') {
                    directiveElement.unbind('dblclick');
//                     directiveElement.unbind('contextmenu');
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
                starttime: item.model.from._d,
                endtime: item.model.to._d
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
            $scope.options.fromDate = $scope.dataService.viewTimeSpan.from;
            $scope.options.toDate = $scope.dataService.viewTimeSpan.to;
            var fullTimespanInMinutes = ($scope.options.toDate - $scope.options.fromDate) / (60 * 1000);

            if(fullTimespanInMinutes > 28*24*60) {
                $scope.options.viewScale = '7 days';
            } else if(fullTimespanInMinutes > 14*24*60) {
                $scope.options.viewScale = '1 day';
            } else if(fullTimespanInMinutes > 7*24*60) {
                $scope.options.viewScale = '6 hours';
            } else if(fullTimespanInMinutes > 2*24*60) {
                $scope.options.viewScale = '3 hours';
            } else if(fullTimespanInMinutes > 12*60) {
                $scope.options.viewScale = '1 hours';
            } else if(fullTimespanInMinutes > 3*60) {
                $scope.options.viewScale = '30 minutes';
            } else {
                $scope.options.viewScale = '15 minutes';
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
                            classes: 'task-status-' + task.status,
                            movable: $.inArray(task.status_id, editableTaskStatusIds) > -1
                        };

                        if(task.id == dataService.selected_task_id) {
                            rowTask.classes += ' task-selected-task';
                        }

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
    $scope.$watch('dataService.selected_task_id', updateGanttData);
    $scope.$watch('dataService.tasks', updateGanttData);
    $scope.$watch('dataService.resources', updateGanttData);
    $scope.$watch('dataService.resourceClaims', updateGanttData);
    $scope.$watch('dataService.resourceGroups', updateGanttData);
    $scope.$watch('dataService.resourceGroupMemberships', updateGanttData);
    $scope.$watch('dataService.filteredTaskDict', updateGanttData);
    $scope.$watch('dataService.momProjectsDict', updateGanttData);
    $scope.$watch('dataService.viewTimeSpan', updateGanttData, true);
    $scope.$watch('dataService.taskChangeCntr', updateGanttData);
    $scope.$watch('dataService.lofarTime', function() {
        if($scope.dataService.lofarTime.getSeconds() % 5 == 0) {
            $scope.options.currentDateValue= $scope.dataService.lofarTime;}});
}
]);
