# moonshine.DefaultApi

All URIs are relative to *https://127.0.0.1:9000*

Method | HTTP request | Description
------------- | ------------- | -------------
[**command_job**](DefaultApi.md#command_job) | **PUT** /jobs/{id} | Send a Command to a Job
[**create_job_for_implant**](DefaultApi.md#create_job_for_implant) | **POST** /implants/{id}/jobs | Create Job For Implant
[**create_listener**](DefaultApi.md#create_listener) | **POST** /listeners | Create Listener
[**create_service**](DefaultApi.md#create_service) | **POST** /services | Create Service
[**create_task_for_implant**](DefaultApi.md#create_task_for_implant) | **POST** /implants/{id}/tasks | Create Task For Implant
[**delete_implant**](DefaultApi.md#delete_implant) | **DELETE** /implants/{id} | Delete Implant
[**delete_listener**](DefaultApi.md#delete_listener) | **DELETE** /listeners/{id} | Delete Listener
[**delete_service**](DefaultApi.md#delete_service) | **DELETE** /services/{id} | Delete Service
[**download_implant**](DefaultApi.md#download_implant) | **GET** /listeners/{id}/artifacts/{operating_system}/{architecture}/{type} | Download implant artifact
[**get_implant**](DefaultApi.md#get_implant) | **GET** /implants/{id} | Get Implant by Id
[**get_implants**](DefaultApi.md#get_implants) | **GET** /implants/offset/{offset}/limit/{limit} | Get Implants
[**get_job**](DefaultApi.md#get_job) | **GET** /jobs/{id} | Get Job by Id
[**get_jobs_for_implant**](DefaultApi.md#get_jobs_for_implant) | **GET** /implants/{id}/jobs/offset/{offset}/limit/{limit} | Get Jobs for Implant
[**get_jobs_for_implant_with_status**](DefaultApi.md#get_jobs_for_implant_with_status) | **GET** /implants/{id}/jobs/status/{status}/offset/{offset}/limit/{limit} | Get Jobs for Implant By Status
[**get_listener**](DefaultApi.md#get_listener) | **GET** /listeners/{id} | Get Listener by Id
[**get_listeners**](DefaultApi.md#get_listeners) | **GET** /listeners/offset/{offset}/limit/{limit} | Get Listeners
[**get_service**](DefaultApi.md#get_service) | **GET** /services/{id} | Get Service by Id
[**get_services**](DefaultApi.md#get_services) | **GET** /services/offset/{offset}/limit/{limit} | Get Services
[**get_task**](DefaultApi.md#get_task) | **GET** /tasks/{id} | Get Task by Id
[**get_tasks_for_implant**](DefaultApi.md#get_tasks_for_implant) | **GET** /implants/{id}/tasks/offset/{offset}/limit/{limit} | Get Tasks for Implant
[**get_tasks_for_implant_with_status**](DefaultApi.md#get_tasks_for_implant_with_status) | **GET** /implants/{id}/tasks/status/{status}/offset/{offset}/limit/{limit} | Get Tasks for Implant By Status
[**start_listener**](DefaultApi.md#start_listener) | **GET** /listeners/{id}/start | Start Listener by Id
[**start_service**](DefaultApi.md#start_service) | **GET** /services/{id}/start | Start Service by Id
[**stop_job**](DefaultApi.md#stop_job) | **DELETE** /jobs/{id} | Stop Job by Id
[**stop_listener**](DefaultApi.md#stop_listener) | **GET** /listeners/{id}/stop | Stop Listener by Id
[**stop_service**](DefaultApi.md#stop_service) | **GET** /services/{id}/stop | Stop Service by Id
[**websocket**](DefaultApi.md#websocket) | **GET** /ws | 


# **command_job**
> Status command_job(id, command)

Send a Command to a Job

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.command import Command
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    id = "id_example" # str | Job Identifier
    command = Command(
        command="command_example",
        data="data_example",
    ) # Command | 

    # example passing only required values which don't have defaults set
    try:
        # Send a Command to a Job
        api_response = api_instance.command_job(id, command)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->command_job: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **str**| Job Identifier |
 **command** | [**Command**](Command.md)|  |

### Return type

[**Status**](Status.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: application/json
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**404** | Not Found |  -  |
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **create_job_for_implant**
> Job create_job_for_implant(id, job)

Create Job For Implant

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.job import Job
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    id = "id_example" # str | Implant Identifier
    job = Job(
        id="id_example",
        name="name_example",
        status=JobStatusInt32(0),
        implant="implant_example",
        package="package_example",
        arguments=[
            Argument(
                data="data_example",
            ),
        ],
        success=JobSuccessInt32(-1),
        output="",
    ) # Job | 

    # example passing only required values which don't have defaults set
    try:
        # Create Job For Implant
        api_response = api_instance.create_job_for_implant(id, job)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->create_job_for_implant: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **str**| Implant Identifier |
 **job** | [**Job**](Job.md)|  |

### Return type

[**Job**](Job.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: application/json
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**404** | Not Found |  -  |
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **create_listener**
> Listener create_listener(listener)

Create Listener

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.listener import Listener
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    listener = Listener(
        id="id_example",
        status=0,
        name="name_example",
        package="package_example",
        implant_package="implant_package_example",
        implant_connection_string="implant_connection_string_example",
        arguments=[
            Argument(
                data="data_example",
            ),
        ],
    ) # Listener | 

    # example passing only required values which don't have defaults set
    try:
        # Create Listener
        api_response = api_instance.create_listener(listener)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->create_listener: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **listener** | [**Listener**](Listener.md)|  |

### Return type

[**Listener**](Listener.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: application/json
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**404** | Not Found |  -  |
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **create_service**
> Service create_service(service)

Create Service

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.service import Service
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    service = Service(
        id="id_example",
        status=0,
        name="name_example",
        package="package_example",
        arguments=[
            Argument(
                data="data_example",
            ),
        ],
    ) # Service | 

    # example passing only required values which don't have defaults set
    try:
        # Create Service
        api_response = api_instance.create_service(service)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->create_service: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **service** | [**Service**](Service.md)|  |

### Return type

[**Service**](Service.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: application/json
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**404** | Not Found |  -  |
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **create_task_for_implant**
> Task create_task_for_implant(id, task)

Create Task For Implant

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.task import Task
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    id = "id_example" # str | Implant Identifier
    task = Task(
        id="id_example",
        status=TaskStatusInt32(0),
        implant="w8q6zgckec",
        script="",
        success=TaskSuccessInt32(-1),
        output="",
    ) # Task | 

    # example passing only required values which don't have defaults set
    try:
        # Create Task For Implant
        api_response = api_instance.create_task_for_implant(id, task)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->create_task_for_implant: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **str**| Implant Identifier |
 **task** | [**Task**](Task.md)|  |

### Return type

[**Task**](Task.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: application/json
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**404** | Not Found |  -  |
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **delete_implant**
> Status delete_implant(id)

Delete Implant

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    id = "id_example" # str | Implant Identifier

    # example passing only required values which don't have defaults set
    try:
        # Delete Implant
        api_response = api_instance.delete_implant(id)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->delete_implant: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **str**| Implant Identifier |

### Return type

[**Status**](Status.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **delete_listener**
> Status delete_listener(id)

Delete Listener

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    id = "id_example" # str | Listener Identifier

    # example passing only required values which don't have defaults set
    try:
        # Delete Listener
        api_response = api_instance.delete_listener(id)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->delete_listener: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **str**| Listener Identifier |

### Return type

[**Status**](Status.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**404** | Not Found |  -  |
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **delete_service**
> Status delete_service(id)

Delete Service

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    id = "id_example" # str | Service Identifier

    # example passing only required values which don't have defaults set
    try:
        # Delete Service
        api_response = api_instance.delete_service(id)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->delete_service: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **str**| Service Identifier |

### Return type

[**Status**](Status.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **download_implant**
> file_type download_implant(id, operating_system, architecture, type)

Download implant artifact

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.architecture_string import ArchitectureString
from moonshine.model.artifact_type_string import ArtifactTypeString
from moonshine.model.operating_system_string import OperatingSystemString
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    id = "id_example" # str | Listener Identifier
    operating_system = OperatingSystemString("windows") # OperatingSystemString | Operating System
    architecture = ArchitectureString("i386") # ArchitectureString | Architecture
    type = ArtifactTypeString("exe") # ArtifactTypeString | Artifact Type

    # example passing only required values which don't have defaults set
    try:
        # Download implant artifact
        api_response = api_instance.download_implant(id, operating_system, architecture, type)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->download_implant: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **str**| Listener Identifier |
 **operating_system** | **OperatingSystemString**| Operating System |
 **architecture** | **ArchitectureString**| Architecture |
 **type** | **ArtifactTypeString**| Artifact Type |

### Return type

**file_type**

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json, application/octet-stream


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**404** | Not Found |  -  |
**500** | Internal Server Error |  -  |
**400** | Bad Request |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **get_implant**
> Implant get_implant(id)

Get Implant by Id

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.implant import Implant
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    id = "id_example" # str | Implant Identifier

    # example passing only required values which don't have defaults set
    try:
        # Get Implant by Id
        api_response = api_instance.get_implant(id)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->get_implant: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **str**| Implant Identifier |

### Return type

[**Implant**](Implant.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**404** | Not Found |  -  |
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **get_implants**
> ImplantsPage get_implants(offset, limit)

Get Implants

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.implants_page import ImplantsPage
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    offset = 0 # int | 
    limit = 0 # int | 

    # example passing only required values which don't have defaults set
    try:
        # Get Implants
        api_response = api_instance.get_implants(offset, limit)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->get_implants: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **offset** | **int**|  |
 **limit** | **int**|  |

### Return type

[**ImplantsPage**](ImplantsPage.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **get_job**
> Job get_job(id)

Get Job by Id

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.job import Job
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    id = "id_example" # str | Job Identifier

    # example passing only required values which don't have defaults set
    try:
        # Get Job by Id
        api_response = api_instance.get_job(id)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->get_job: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **str**| Job Identifier |

### Return type

[**Job**](Job.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**404** | Not Found |  -  |
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **get_jobs_for_implant**
> JobsPage get_jobs_for_implant(id, offset, limit)

Get Jobs for Implant

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.jobs_page import JobsPage
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    id = "id_example" # str | Implant Identifier
    offset = 0 # int | 
    limit = 0 # int | 

    # example passing only required values which don't have defaults set
    try:
        # Get Jobs for Implant
        api_response = api_instance.get_jobs_for_implant(id, offset, limit)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->get_jobs_for_implant: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **str**| Implant Identifier |
 **offset** | **int**|  |
 **limit** | **int**|  |

### Return type

[**JobsPage**](JobsPage.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**404** | Not Found |  -  |
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **get_jobs_for_implant_with_status**
> TasksPage get_jobs_for_implant_with_status(id, status, offset, limit)

Get Jobs for Implant By Status

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.tasks_page import TasksPage
from moonshine.model.job_status_int32 import JobStatusInt32
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    id = "id_example" # str | Implant Identifier
    status = JobStatusInt32(0) # JobStatusInt32 | Status Type
    offset = 0 # int | 
    limit = 0 # int | 

    # example passing only required values which don't have defaults set
    try:
        # Get Jobs for Implant By Status
        api_response = api_instance.get_jobs_for_implant_with_status(id, status, offset, limit)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->get_jobs_for_implant_with_status: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **str**| Implant Identifier |
 **status** | **JobStatusInt32**| Status Type |
 **offset** | **int**|  |
 **limit** | **int**|  |

### Return type

[**TasksPage**](TasksPage.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**404** | Not Found |  -  |
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **get_listener**
> Listener get_listener(id)

Get Listener by Id

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.listener import Listener
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    id = "id_example" # str | Listener Identifier

    # example passing only required values which don't have defaults set
    try:
        # Get Listener by Id
        api_response = api_instance.get_listener(id)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->get_listener: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **str**| Listener Identifier |

### Return type

[**Listener**](Listener.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**404** | Not Found |  -  |
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **get_listeners**
> ListenersPage get_listeners(offset, limit)

Get Listeners

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.listeners_page import ListenersPage
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    offset = 0 # int | 
    limit = 0 # int | 

    # example passing only required values which don't have defaults set
    try:
        # Get Listeners
        api_response = api_instance.get_listeners(offset, limit)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->get_listeners: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **offset** | **int**|  |
 **limit** | **int**|  |

### Return type

[**ListenersPage**](ListenersPage.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **get_service**
> Service get_service(id)

Get Service by Id

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.service import Service
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    id = "id_example" # str | Service Identifier

    # example passing only required values which don't have defaults set
    try:
        # Get Service by Id
        api_response = api_instance.get_service(id)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->get_service: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **str**| Service Identifier |

### Return type

[**Service**](Service.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**404** | Not Found |  -  |
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **get_services**
> ServicesPage get_services(offset, limit)

Get Services

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.services_page import ServicesPage
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    offset = 0 # int | 
    limit = 0 # int | 

    # example passing only required values which don't have defaults set
    try:
        # Get Services
        api_response = api_instance.get_services(offset, limit)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->get_services: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **offset** | **int**|  |
 **limit** | **int**|  |

### Return type

[**ServicesPage**](ServicesPage.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **get_task**
> Task get_task(id)

Get Task by Id

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.task import Task
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    id = "id_example" # str | Task Identifier

    # example passing only required values which don't have defaults set
    try:
        # Get Task by Id
        api_response = api_instance.get_task(id)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->get_task: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **str**| Task Identifier |

### Return type

[**Task**](Task.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**404** | Not Found |  -  |
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **get_tasks_for_implant**
> TasksPage get_tasks_for_implant(id, offset, limit)

Get Tasks for Implant

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.tasks_page import TasksPage
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    id = "id_example" # str | Implant Identifier
    offset = 0 # int | 
    limit = 0 # int | 

    # example passing only required values which don't have defaults set
    try:
        # Get Tasks for Implant
        api_response = api_instance.get_tasks_for_implant(id, offset, limit)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->get_tasks_for_implant: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **str**| Implant Identifier |
 **offset** | **int**|  |
 **limit** | **int**|  |

### Return type

[**TasksPage**](TasksPage.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**404** | Not Found |  -  |
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **get_tasks_for_implant_with_status**
> TasksPage get_tasks_for_implant_with_status(id, status, offset, limit)

Get Tasks for Implant By Status

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.tasks_page import TasksPage
from moonshine.model.task_status_int32 import TaskStatusInt32
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    id = "id_example" # str | Implant Identifier
    status = TaskStatusInt32(0) # TaskStatusInt32 | Status Type
    offset = 0 # int | 
    limit = 0 # int | 

    # example passing only required values which don't have defaults set
    try:
        # Get Tasks for Implant By Status
        api_response = api_instance.get_tasks_for_implant_with_status(id, status, offset, limit)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->get_tasks_for_implant_with_status: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **str**| Implant Identifier |
 **status** | **TaskStatusInt32**| Status Type |
 **offset** | **int**|  |
 **limit** | **int**|  |

### Return type

[**TasksPage**](TasksPage.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**404** | Not Found |  -  |
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **start_listener**
> Status start_listener(id)

Start Listener by Id

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    id = "id_example" # str | Listener Identifier

    # example passing only required values which don't have defaults set
    try:
        # Start Listener by Id
        api_response = api_instance.start_listener(id)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->start_listener: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **str**| Listener Identifier |

### Return type

[**Status**](Status.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**404** | Not Found |  -  |
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **start_service**
> Status start_service(id)

Start Service by Id

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    id = "id_example" # str | Service Identifier

    # example passing only required values which don't have defaults set
    try:
        # Start Service by Id
        api_response = api_instance.start_service(id)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->start_service: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **str**| Service Identifier |

### Return type

[**Status**](Status.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**404** | Not Found |  -  |
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **stop_job**
> Status stop_job(id)

Stop Job by Id

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    id = "id_example" # str | Job Identifier

    # example passing only required values which don't have defaults set
    try:
        # Stop Job by Id
        api_response = api_instance.stop_job(id)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->stop_job: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **str**| Job Identifier |

### Return type

[**Status**](Status.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**404** | Not Found |  -  |
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **stop_listener**
> Status stop_listener(id)

Stop Listener by Id

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    id = "id_example" # str | Listener Identifier

    # example passing only required values which don't have defaults set
    try:
        # Stop Listener by Id
        api_response = api_instance.stop_listener(id)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->stop_listener: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **str**| Listener Identifier |

### Return type

[**Status**](Status.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**404** | Not Found |  -  |
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **stop_service**
> Status stop_service(id)

Stop Service by Id

### Example


```python
import time
import moonshine
from moonshine.api import default_api
from moonshine.model.status import Status
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)
    id = "id_example" # str | Service Identifier

    # example passing only required values which don't have defaults set
    try:
        # Stop Service by Id
        api_response = api_instance.stop_service(id)
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->stop_service: %s\n" % e)
```


### Parameters

Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
 **id** | **str**| Service Identifier |

### Return type

[**Status**](Status.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**404** | Not Found |  -  |
**500** | Internal Server Error |  -  |
**200** | OK |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **websocket**
> str websocket()



### Example


```python
import time
import moonshine
from moonshine.api import default_api
from pprint import pprint
# Defining the host is optional and defaults to https://127.0.0.1:9000
# See configuration.py for a list of all supported configuration parameters.
configuration = moonshine.Configuration(
    host = "https://127.0.0.1:9000"
)


# Enter a context with an instance of the API client
with moonshine.ApiClient() as api_client:
    # Create an instance of the API class
    api_instance = default_api.DefaultApi(api_client)

    # example, this endpoint has no required or optional parameters
    try:
        api_response = api_instance.websocket()
        pprint(api_response)
    except moonshine.ApiException as e:
        print("Exception when calling DefaultApi->websocket: %s\n" % e)
```


### Parameters
This endpoint does not need any parameter.

### Return type

**str**

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: text/plain


### HTTP response details

| Status code | Description | Response headers |
|-------------|-------------|------------------|
**200** | success |  -  |

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

