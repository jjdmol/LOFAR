from django.conf.urls.defaults import *
from mysite.views import current_datetime, hours_ahead
import mysite.shm as shm
import mysite.data as data

urlpatterns = patterns('',
                       # Example:
                       #(r'^mysite/', include('mysite.foo.urls')),
                       
                       # Uncomment this for admin:
                       #(r'^admin/', include('django.contrib.admin.urls')),
                       (r'(?P<path>.*css)$','django.views.static.serve', {'document_root': '/home/lofartest/SHM/Components/WebInterface/testing/djstart/mysite'}),
                       (r'(?P<path>.*png)$','django.views.static.serve', {'document_root': '/home/lofartest/SHM/Components/WebInterface/testing/djstart/mysite'}),
                       (r'^shm/$',shm.index),
                       (r'^shm/data/$', data.defaultpage),
                       (r'^shm/data/xst/(?P<station>cs.*)/image/thumb/(?P<plotnumber>\d+)/$','data.xst'),
                       (r'^shm/data/xst/(?P<station>cs.*)/image/(?P<plotnumber>\d+)/$','data.xst'),
                       (r'^shm/data/xst/(?P<station>cs.*)/image/$','data.xst'),
                       (r'^shm/data/xst/(?P<station>cs.*)/fetch/$','data.xst'),
                       (r'^shm/data/xst/(?P<station>cs.*)/$', 'data.xst'),
                       (r'^shm/data/sky/(?P<station>cs.*)/image/thumb/(?P<plotnumber>\d+)/$',data.sky),
                       (r'^shm/data/sky/(?P<station>cs.*)/image/(?P<plotnumber>\d+)/$',data.sky),
                       (r'^shm/data/sky/(?P<station>cs.*)/image/$',data.sky),
                       (r'^shm/data/sky/(?P<station>cs.*)/fetch/$',data.sky),
                       (r'^shm/data/sky/(?P<station>cs.*)/$', data.sky),
                       (r'^shm/data/sst/(?P<station>cs.*)/image/thumb/(?P<plotnumber>\d+)/$',data.sst),
                       (r'^shm/data/sst/(?P<station>cs.*)/image/(?P<plotnumber>\d+)/$',data.sst),
                       (r'^shm/data/sst/(?P<station>cs.*)/image/$',data.sst),
                       (r'^shm/data/sst/(?P<station>cs.*)/fetch/$','data.sst'),
                       (r'^shm/data/sst/(?P<station>cs.*)/$', 'data.sst'),
                       (r'^shm/data/rsp/(?P<station>cs.*)/image/(?P<plotnumber>\d+)/$','data.rsp'),
                       (r'^shm/data/rsp/(?P<station>cs.*)/image/$','data.rsp'),
                       (r'^shm/data/rsp/(?P<station>cs.*)/fetch/$','data.rsp'),
                       (r'^shm/data/rsp/(?P<station>cs.*)/$', 'data.rsp'),
                       )
